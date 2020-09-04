//---------------------------------------------------------------------------//
// Copyright (c) 2011-2020 Dominik Charousset
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#include <nil/actor/io/network/default_multiplexer.hpp>

#include <utility>

#include <nil/actor/config.hpp>
#include <nil/actor/defaults.hpp>
#include <nil/actor/optional.hpp>
#include <nil/actor/make_counted.hpp>
#include <nil/actor/spawner_config.hpp>

#include <nil/actor/io/broker.hpp>
#include <nil/actor/io/middleman.hpp>
#include <nil/actor/io/network/protocol.hpp>
#include <nil/actor/io/network/interfaces.hpp>
#include <nil/actor/io/network/scribe_impl.hpp>
#include <nil/actor/io/network/doorman_impl.hpp>
#include <nil/actor/io/network/datagram_servant_impl.hpp>

#include <nil/actor/detail/call_cfun.hpp>
#include <nil/actor/detail/socket_guard.hpp>

#include <nil/actor/scheduler/abstract_coordinator.hpp>

#ifdef BOOST_OS_WINDOWS_AVAILABLE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif    // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifdef ACTOR_MINGW
#undef _WIN32_WINNT
#undef WINVER
#define _WIN32_WINNT WindowsVista
#define WINVER WindowsVista
#include <w32api.h>
#endif
#include <io.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <ws2tcpip.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#ifdef ACTOR_POLL_MULTIPLEXER
#include <poll.h>
#elif defined(ACTOR_EPOLL_MULTIPLEXER)
#include <sys/epoll.h>
#else
#error "neither ACTOR_POLL_MULTIPLEXER nor ACTOR_EPOLL_MULTIPLEXER defined"
#endif

#endif

using std::string;

namespace {

    // Save ourselves some typing.
    constexpr auto ipv4 = nil::actor::io::network::protocol::ipv4;
    constexpr auto ipv6 = nil::actor::io::network::protocol::ipv6;

    auto addr_of(sockaddr_in &what) -> decltype(what.sin_addr) & {
        return what.sin_addr;
    }

    auto family_of(sockaddr_in &what) -> decltype(what.sin_family) & {
        return what.sin_family;
    }

    auto port_of(sockaddr_in &what) -> decltype(what.sin_port) & {
        return what.sin_port;
    }

    auto addr_of(sockaddr_in6 &what) -> decltype(what.sin6_addr) & {
        return what.sin6_addr;
    }

    auto family_of(sockaddr_in6 &what) -> decltype(what.sin6_family) & {
        return what.sin6_family;
    }

    auto port_of(sockaddr_in6 &what) -> decltype(what.sin6_port) & {
        return what.sin6_port;
    }

}    // namespace

namespace nil {
    namespace actor {
        namespace io {
            namespace network {

// poll vs epoll backend
#ifdef ACTOR_POLL_MULTIPLEXER
#ifndef POLLRDHUP
#define POLLRDHUP POLLHUP
#endif
#ifndef POLLPRI
#define POLLPRI POLLIN
#endif
#ifdef BOOST_OS_WINDOWS_AVAILABLE
                // From the MSDN: If the POLLPRI flag is set on a socket for the Microsoft
                //                Winsock provider, the WSAPoll function will fail.
                const event_mask_type input_mask = POLLIN;
#else
                const event_mask_type input_mask = POLLIN | POLLPRI;
#endif
                const event_mask_type error_mask = POLLRDHUP | POLLERR | POLLHUP | POLLNVAL;
                const event_mask_type output_mask = POLLOUT;
#else
                const event_mask_type input_mask = EPOLLIN;
                const event_mask_type error_mask = EPOLLRDHUP | EPOLLERR | EPOLLHUP;
                const event_mask_type output_mask = EPOLLOUT;
#endif

                // -- Platform-dependent abstraction over epoll() or poll() --------------------

#ifdef ACTOR_EPOLL_MULTIPLEXER

                // In this implementation, shadow_ is the number of sockets we have
                // registered to epoll.

                default_multiplexer::default_multiplexer(spawner *sys) :
                    multiplexer(sys), epollfd_(invalid_native_socket), shadow_(1), pipe_reader_(*this), servant_ids_(0),
                    max_throughput_(0) {
                    init();
                    epollfd_ = epoll_create1(EPOLL_CLOEXEC);
                    if (epollfd_ == -1) {
                        ACTOR_LOG_ERROR("epoll_create1: " << strerror(errno));
                        exit(errno);
                    }
                    // handle at most 64 events at a time
                    pollset_.resize(64);
                    pipe_ = create_pipe();
                    pipe_reader_.init(pipe_.first);
                    epoll_event ee;
                    ee.events = input_mask;
                    ee.data.ptr = &pipe_reader_;
                    if (epoll_ctl(epollfd_, EPOLL_CTL_ADD, pipe_reader_.fd(), &ee) < 0) {
                        ACTOR_LOG_ERROR("epoll_ctl: " << strerror(errno));
                        exit(errno);
                    }
                }

                bool default_multiplexer::poll_once_impl(bool block) {
                    ACTOR_LOG_TRACE("epoll()-based multiplexer");
                    ACTOR_ASSERT(block == false || internally_posted_.empty());
                    // Keep running in case of `EINTR`.
                    for (;;) {
                        int presult =
                            epoll_wait(epollfd_, pollset_.data(), static_cast<int>(pollset_.size()), block ? -1 : 0);
                        ACTOR_LOG_DEBUG("epoll_wait() on" << shadow_ << "sockets reported" << presult << "event(s)");
                        if (presult < 0) {
                            switch (errno) {
                                case EINTR: {
                                    // a signal was caught
                                    // just try again
                                    continue;
                                }
                                default: {
                                    perror("epoll_wait() failed");
                                    ACTOR_CRITICAL("epoll_wait() failed");
                                }
                            }
                        }
                        if (presult == 0)
                            return false;
                        auto iter = pollset_.begin();
                        auto last = iter + presult;
                        for (; iter != last; ++iter) {
                            auto ptr = reinterpret_cast<event_handler *>(iter->data.ptr);
                            auto fd = ptr ? ptr->fd() : pipe_.first;
                            handle_socket_event(fd, static_cast<int>(iter->events), ptr);
                        }
                        handle_internal_events();
                        return true;
                    }
                }

                void default_multiplexer::run() {
                    ACTOR_LOG_TRACE("epoll()-based multiplexer");
                    while (shadow_ > 0)
                        poll_once(true);
                }

                void default_multiplexer::handle(const default_multiplexer::event &e) {
                    ACTOR_LOG_TRACE("e.fd = " << ACTOR_ARG(e.fd) << ", mask = " << ACTOR_ARG(e.mask));
                    // ptr is only allowed to nullptr if fd is our pipe
                    // read handle which is only registered for input
                    ACTOR_ASSERT(e.ptr != nullptr || e.fd == pipe_.first);
                    if (e.ptr && e.ptr->eventbf() == e.mask) {
                        // nop
                        return;
                    }
                    auto old = e.ptr ? e.ptr->eventbf() : input_mask;
                    if (e.ptr) {
                        e.ptr->eventbf(e.mask);
                    }
                    epoll_event ee;
                    ee.events = static_cast<uint32_t>(e.mask);
                    ee.data.ptr = e.ptr;
                    int op;
                    if (e.mask == 0) {
                        ACTOR_LOG_DEBUG("attempt to remove socket " << ACTOR_ARG(e.fd) << " from epoll");
                        op = EPOLL_CTL_DEL;
                        --shadow_;
                    } else if (old == 0) {
                        ACTOR_LOG_DEBUG("attempt to add socket " << ACTOR_ARG(e.fd) << " to epoll");
                        op = EPOLL_CTL_ADD;
                        ++shadow_;
                    } else {
                        ACTOR_LOG_DEBUG("modify epoll event mask for socket " << ACTOR_ARG(e.fd) << ": " << ACTOR_ARG(old)
                                                                            << " -> " << ACTOR_ARG(e.mask));
                        op = EPOLL_CTL_MOD;
                    }
                    if (epoll_ctl(epollfd_, op, e.fd, &ee) < 0) {
                        switch (last_socket_error()) {
                            // supplied file descriptor is already registered
                            case EEXIST:
                                ACTOR_LOG_ERROR("file descriptor registered twice");
                                --shadow_;
                                break;
                            // op was EPOLL_CTL_MOD or EPOLL_CTL_DEL,
                            // and fd is not registered with this epoll instance.
                            case ENOENT:
                                ACTOR_LOG_ERROR(
                                    "cannot delete file descriptor "
                                    "because it isn't registered");
                                if (e.mask == 0) {
                                    ++shadow_;
                                }
                                break;
                            default:
                                ACTOR_LOG_ERROR(strerror(errno));
                                perror("epoll_ctl() failed");
                                ACTOR_CRITICAL("epoll_ctl() failed");
                        }
                    }
                    if (e.ptr) {
                        auto remove_from_loop_if_needed = [&](int flag, operation flag_op) {
                            if ((old & flag) && !(e.mask & flag)) {
                                e.ptr->removed_from_loop(flag_op);
                            }
                        };
                        remove_from_loop_if_needed(input_mask, operation::read);
                        remove_from_loop_if_needed(output_mask, operation::write);
                    }
                }

                size_t default_multiplexer::num_socket_handlers() const noexcept {
                    return shadow_;
                }

#else    // ACTOR_EPOLL_MULTIPLEXER

                // Let's be honest: the API of poll() sucks. When dealing with 1000 sockets
                // and the very last socket in your pollset triggers, you have to traverse
                // all elements only to find a single event. Even worse, poll() does
                // not give you a way of storing a user-defined pointer in the pollset.
                // Hence, you need to find a pointer to the actual object managing the
                // socket. When using a map, your already dreadful O(n) turns into
                // a worst case of O(n * log n). To deal with this nonsense, we have two
                // vectors in this implementation: pollset_ and shadow_. The former
                // stores our pollset, the latter stores our pointers. Both vectors
                // are sorted by the file descriptor. This allows us to quickly,
                // i.e., O(1), access the actual object when handling socket events.

                default_multiplexer::default_multiplexer(spawner *sys) :
                    multiplexer(sys), epollfd_(-1), pipe_reader_(*this), servant_ids_(0) {
                    init();
                    // initial setup
                    pipe_ = create_pipe();
                    pipe_reader_.init(pipe_.first);
                    pollfd pipefd;
                    pipefd.fd = pipe_reader_.fd();
                    pipefd.events = input_mask;
                    pipefd.revents = 0;
                    pollset_.push_back(pipefd);
                    shadow_.push_back(&pipe_reader_);
                }

                bool default_multiplexer::poll_once_impl(bool block) {
                    ACTOR_LOG_TRACE("poll()-based multiplexer");
                    ACTOR_ASSERT(block == false || internally_posted_.empty());
                    // we store the results of poll() in a separate vector , because
                    // altering the pollset while traversing it is not exactly a
                    // bright idea ...
                    struct fd_event {
                        native_socket fd;      // our file descriptor
                        short mask;            // the event mask returned by poll()
                        event_handler *ptr;    // nullptr in case of a pipe event
                    };
                    std::vector<fd_event> poll_res;
                    for (;;) {
                        int presult;
#ifdef BOOST_OS_WINDOWS_AVAILABLE
                        presult = ::WSAPoll(pollset_.data(), static_cast<ULONG>(pollset_.size()), block ? -1 : 0);
#else
                        presult = ::poll(pollset_.data(), static_cast<nfds_t>(pollset_.size()), block ? -1 : 0);
#endif
                        if (presult < 0) {
                            switch (last_socket_error()) {
                                case EINTR: {
                                    ACTOR_LOG_DEBUG("received EINTR, try again");
                                    // a signal was caught
                                    // just try again
                                    break;
                                }
                                case ENOMEM: {
                                    ACTOR_LOG_ERROR("poll() failed for reason ENOMEM");
                                    // there's not much we can do other than try again
                                    // in hope someone else releases memory
                                    break;
                                }
                                default: {
                                    perror("poll() failed");
                                    ACTOR_CRITICAL("poll() failed");
                                }
                            }
                            continue;    // rinse and repeat
                        }
                        ACTOR_LOG_DEBUG("poll() on" << pollset_.size() << "sockets reported" << presult << "event(s)");
                        if (presult == 0)
                            return false;
                        // scan pollset for events first, because we might alter pollset_
                        // while running callbacks (not a good idea while traversing it)
                        ACTOR_LOG_DEBUG("scan pollset for socket events");
                        for (size_t i = 0; i < pollset_.size() && presult > 0; ++i) {
                            auto &pfd = pollset_[i];
                            if (pfd.revents != 0) {
                                ACTOR_LOG_DEBUG("event on socket:" << ACTOR_ARG(pfd.fd) << ACTOR_ARG(pfd.revents));
                                poll_res.push_back({pfd.fd, pfd.revents, shadow_[i]});
                                pfd.revents = 0;
                                --presult;    // stop as early as possible
                            }
                        }
                        ACTOR_LOG_DEBUG(ACTOR_ARG(poll_res.size()));
                        for (auto &e : poll_res) {
                            // we try to read/write as much as possible by ignoring
                            // error states as long as there are still valid
                            // operations possible on the socket
                            handle_socket_event(e.fd, e.mask, e.ptr);
                        }
                        poll_res.clear();
                        handle_internal_events();
                        return true;
                    }
                }

                void default_multiplexer::run() {
                    ACTOR_LOG_TRACE("poll()-based multiplexer:" << ACTOR_ARG(input_mask) << ACTOR_ARG(output_mask)
                                                              << ACTOR_ARG(error_mask));
                    while (!pollset_.empty())
                        poll_once(true);
                }

                void default_multiplexer::handle(const default_multiplexer::event &e) {
                    ACTOR_ASSERT(e.fd != invalid_native_socket);
                    ACTOR_ASSERT(pollset_.size() == shadow_.size());
                    ACTOR_LOG_TRACE(ACTOR_ARG(e.fd) << ACTOR_ARG(e.mask));
                    auto last = pollset_.end();
                    auto i = std::lower_bound(pollset_.begin(), last, e.fd,
                                              [](const pollfd &lhs, native_socket rhs) { return lhs.fd < rhs; });
                    pollfd new_element;
                    new_element.fd = e.fd;
                    new_element.events = static_cast<short>(e.mask);
                    new_element.revents = 0;
                    int old_mask = 0;
                    if (e.ptr != nullptr) {
                        old_mask = e.ptr->eventbf();
                        e.ptr->eventbf(e.mask);
                    }
                    // calculate shadow of i
                    multiplexer_poll_shadow_data::iterator j;
                    if (i == last) {
                        j = shadow_.end();
                    } else {
                        j = shadow_.begin();
                        std::advance(j, distance(pollset_.begin(), i));
                    }
                    // modify vectors
                    if (i == last) {    // append
                        if (e.mask != 0) {
                            pollset_.push_back(new_element);
                            shadow_.push_back(e.ptr);
                        }
                    } else if (i->fd == e.fd) {    // modify
                        if (e.mask == 0) {
                            // delete item
                            pollset_.erase(i);
                            shadow_.erase(j);
                        } else {
                            // update event mask of existing entry
                            ACTOR_ASSERT(*j == e.ptr);
                            i->events = static_cast<short>(e.mask);
                        }
                        if (e.ptr != nullptr) {
                            auto remove_from_loop_if_needed = [&](int flag, operation flag_op) {
                                if (((old_mask & flag) != 0) && ((e.mask & flag) == 0)) {
                                    e.ptr->removed_from_loop(flag_op);
                                }
                            };
                            remove_from_loop_if_needed(input_mask, operation::read);
                            remove_from_loop_if_needed(output_mask, operation::write);
                        }
                    } else {    // insert at iterator pos
                        pollset_.insert(i, new_element);
                        shadow_.insert(j, e.ptr);
                    }
                }

                size_t default_multiplexer::num_socket_handlers() const noexcept {
                    return pollset_.size();
                }

#endif    // ACTOR_EPOLL_MULTIPLEXER

                // -- Helper functions for defining bitmasks of event handlers -----------------

                int add_flag(operation op, int bf) {
                    switch (op) {
                        case operation::read:
                            return bf | input_mask;
                        case operation::write:
                            return bf | output_mask;
                        case operation::propagate_error:
                            ACTOR_LOG_ERROR("unexpected operation");
                            break;
                    }
                    // weird stuff going on
                    return 0;
                }

                int del_flag(operation op, int bf) {
                    switch (op) {
                        case operation::read:
                            return bf & ~input_mask;
                        case operation::write:
                            return bf & ~output_mask;
                        case operation::propagate_error:
                            ACTOR_LOG_ERROR("unexpected operation");
                            break;
                    }
                    // weird stuff going on
                    return 0;
                }

                // -- Platform-independent parts of the default_multiplexer --------------------

                bool default_multiplexer::try_run_once() {
                    return poll_once(false);
                }

                void default_multiplexer::run_once() {
                    poll_once(true);
                }

                void default_multiplexer::add(operation op, native_socket fd, event_handler *ptr) {
                    ACTOR_ASSERT(fd != invalid_native_socket);
                    // ptr == nullptr is only allowed to store our pipe read handle
                    // and the pipe read handle is added in the ctor (not allowed here)
                    ACTOR_ASSERT(ptr != nullptr);
                    ACTOR_LOG_TRACE(ACTOR_ARG(op) << ACTOR_ARG(fd));
                    new_event(add_flag, op, fd, ptr);
                }

                void default_multiplexer::del(operation op, native_socket fd, event_handler *ptr) {
                    ACTOR_ASSERT(fd != invalid_native_socket);
                    // ptr == nullptr is only allowed when removing our pipe read handle
                    ACTOR_ASSERT(ptr != nullptr || fd == pipe_.first);
                    ACTOR_LOG_TRACE(ACTOR_ARG(op) << ACTOR_ARG(fd));
                    new_event(del_flag, op, fd, ptr);
                }

                void default_multiplexer::wr_dispatch_request(resumable *ptr) {
                    intptr_t ptrval = reinterpret_cast<intptr_t>(ptr);
                    // on windows, we actually have sockets, otherwise we have file handles
#ifdef BOOST_OS_WINDOWS_AVAILABLE
                    auto res = ::send(pipe_.second, reinterpret_cast<socket_send_ptr>(&ptrval), sizeof(ptrval),
                                      no_sigpipe_io_flag);
#else
                    auto res = ::write(pipe_.second, &ptrval, sizeof(ptrval));
#endif
                    if (res <= 0) {
                        // pipe closed, discard resumable
                        intrusive_ptr_release(ptr);
                    } else if (static_cast<size_t>(res) < sizeof(ptrval)) {
                        // must not happen: wrote invalid pointer to pipe
                        std::cerr << "[ACTOR] Fatal error: wrote invalid data to pipe" << std::endl;
                        abort();
                    }
                }

                multiplexer::supervisor_ptr default_multiplexer::make_supervisor() {
                    class impl : public multiplexer::supervisor {
                    public:
                        explicit impl(default_multiplexer *thisptr) : this_(thisptr) {
                            // nop
                        }
                        ~impl() override {
                            auto ptr = this_;
                            ptr->dispatch([=] { ptr->close_pipe(); });
                        }

                    private:
                        default_multiplexer *this_;
                    };
                    return supervisor_ptr {new impl(this)};
                }

                void default_multiplexer::close_pipe() {
                    ACTOR_LOG_TRACE("");
                    del(operation::read, pipe_.first, nullptr);
                }

                void default_multiplexer::handle_socket_event(native_socket fd, int mask, event_handler *ptr) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(fd) << ACTOR_ARG(mask));
                    ACTOR_ASSERT(ptr != nullptr);
                    bool checkerror = true;
                    if ((mask & input_mask) != 0) {
                        checkerror = false;
                        // ignore read events if a previous event caused
                        // this socket to be shut down for reading
                        if (!ptr->read_channel_closed())
                            ptr->handle_event(operation::read);
                    }
                    if ((mask & output_mask) != 0) {
                        checkerror = false;
                        ptr->handle_event(operation::write);
                    }
                    if (checkerror && ((mask & error_mask) != 0)) {
                        ACTOR_LOG_DEBUG("error occured on socket:" << ACTOR_ARG(fd) << ACTOR_ARG(last_socket_error())
                                                                 << ACTOR_ARG(last_socket_error_as_string()));
                        ptr->handle_event(operation::propagate_error);
                        del(operation::read, fd, ptr);
                        del(operation::write, fd, ptr);
                    }
                }

                void default_multiplexer::init() {
#ifdef BOOST_OS_WINDOWS_AVAILABLE
                    WSADATA WinsockData;
                    if (WSAStartup(MAKEWORD(2, 2), &WinsockData) != 0) {
                        ACTOR_CRITICAL("WSAStartup failed");
                    }
#endif
                    namespace sr = defaults::scheduler;
                    max_throughput_ = system().config().scheduler_max_throughput;
                }

                bool default_multiplexer::poll_once(bool block) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(block));
                    if (!internally_posted_.empty()) {
                        // Don't iterate internally_posted_ directly, because resumables can
                        // enqueue new elements into it.
                        std::vector<intrusive_ptr<resumable>> xs;
                        internally_posted_.swap(xs);
                        for (auto &ptr : xs)
                            resume(std::move(ptr));
                        handle_internal_events();
                        // Try to swap back to internall_posted_ to re-use allocated memory.
                        if (internally_posted_.empty()) {
                            xs.swap(internally_posted_);
                            internally_posted_.clear();
                        }
                        poll_once_impl(false);
                        return true;
                    }
                    return poll_once_impl(block);
                }

                void default_multiplexer::resume(intrusive_ptr<resumable> ptr) {
                    ACTOR_LOG_TRACE("");
                    switch (ptr->resume(this, max_throughput_)) {
                        case resumable::resume_later:
                            // Delay resumable until next cycle.
                            internally_posted_.emplace_back(ptr.detach(), false);
                            break;
                        case resumable::shutdown_execution_unit:
                            // Don't touch reference count of shutdown helpers.
                            ptr.detach();
                            break;
                        default:;    // Done. Release reference to resumable.
                    }
                }

                default_multiplexer::~default_multiplexer() {
                    if (epollfd_ != invalid_native_socket)
                        close_socket(epollfd_);
                    // close write handle first
                    close_socket(pipe_.second);
                    // flush pipe before closing it
                    nonblocking(pipe_.first, true);
                    auto ptr = pipe_reader_.try_read_next();
                    while (ptr != nullptr) {
                        scheduler::abstract_coordinator::cleanup_and_release(ptr);
                        ptr = pipe_reader_.try_read_next();
                    }
                    // do cleanup for pipe reader manually, since WSACleanup needs to happen last
                    close_socket(pipe_reader_.fd());
                    pipe_reader_.init(invalid_native_socket);
#ifdef BOOST_OS_WINDOWS_AVAILABLE
                    WSACleanup();
#endif
                }

                void default_multiplexer::exec_later(resumable *ptr) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(ptr));
                    ACTOR_ASSERT(ptr != nullptr);
                    switch (ptr->subtype()) {
                        case resumable::io_actor:
                        case resumable::function_object:
                            if (std::this_thread::get_id() != thread_id())
                                wr_dispatch_request(ptr);
                            else
                                internally_posted_.emplace_back(ptr, false);
                            break;
                        default:
                            system().scheduler().enqueue(ptr);
                    }
                }

                scribe_ptr default_multiplexer::new_scribe(native_socket fd) {
                    ACTOR_LOG_TRACE("");
                    keepalive(fd, true);
                    return make_counted<scribe_impl>(*this, fd);
                }

                expected<scribe_ptr> default_multiplexer::new_tcp_scribe(const std::string &host, uint16_t port) {
                    auto fd = new_tcp_connection(host, port);
                    if (!fd)
                        return std::move(fd.error());
                    return new_scribe(*fd);
                }

                doorman_ptr default_multiplexer::new_doorman(native_socket fd) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(fd));
                    ACTOR_ASSERT(fd != network::invalid_native_socket);
                    return make_counted<doorman_impl>(*this, fd);
                }

                expected<doorman_ptr> default_multiplexer::new_tcp_doorman(uint16_t port, const char *in,
                                                                           bool reuse_addr) {
                    auto fd = new_tcp_acceptor_impl(port, in, reuse_addr);
                    if (fd)
                        return new_doorman(*fd);
                    return std::move(fd.error());
                }

                datagram_servant_ptr default_multiplexer::new_datagram_servant(native_socket fd) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(fd));
                    ACTOR_ASSERT(fd != network::invalid_native_socket);
                    return make_counted<datagram_servant_impl>(*this, fd, next_endpoint_id());
                }

                datagram_servant_ptr default_multiplexer::new_datagram_servant_for_endpoint(native_socket fd,
                                                                                            const ip_endpoint &ep) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(ep));
                    auto ds = new_datagram_servant(fd);
                    ds->add_endpoint(ep, ds->hdl());
                    return ds;
                }

                expected<datagram_servant_ptr> default_multiplexer::new_remote_udp_endpoint(const std::string &host,
                                                                                            uint16_t port) {
                    auto res = new_remote_udp_endpoint_impl(host, port);
                    if (!res)
                        return std::move(res.error());
                    return new_datagram_servant_for_endpoint(res->first, res->second);
                }

                expected<datagram_servant_ptr>
                    default_multiplexer::new_local_udp_endpoint(uint16_t port, const char *in, bool reuse_addr) {
                    auto res = new_local_udp_endpoint_impl(port, in, reuse_addr);
                    if (res)
                        return new_datagram_servant((*res).first);
                    return std::move(res.error());
                }

                int64_t default_multiplexer::next_endpoint_id() {
                    return servant_ids_++;
                }

                void default_multiplexer::handle_internal_events() {
                    ACTOR_LOG_TRACE(ACTOR_ARG2("num-events", events_.size()));
                    for (auto &e : events_)
                        handle(e);
                    events_.clear();
                }

                // -- Related helper functions -------------------------------------------------

                template<int Family>
                bool ip_connect(native_socket fd, const std::string &host, uint16_t port) {
                    ACTOR_LOG_TRACE("Family =" << (Family == AF_INET ? "AF_INET" : "AF_INET6") << ACTOR_ARG(fd)
                                             << ACTOR_ARG(host));
                    static_assert(Family == AF_INET || Family == AF_INET6, "invalid family");
                    using sockaddr_type = typename std::conditional<Family == AF_INET, sockaddr_in, sockaddr_in6>::type;
                    sockaddr_type sa;
                    memset(&sa, 0, sizeof(sockaddr_type));
                    inet_pton(Family, host.c_str(), &addr_of(sa));
                    family_of(sa) = Family;
                    port_of(sa) = htons(port);
                    return connect(fd, reinterpret_cast<const sockaddr *>(&sa), sizeof(sa)) == 0;
                }

                expected<native_socket> new_tcp_connection(const std::string &host, uint16_t port,
                                                           optional<protocol::network> preferred) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(host) << ACTOR_ARG(port) << ACTOR_ARG(preferred));
                    ACTOR_LOG_DEBUG("try to connect to:" << ACTOR_ARG(host) << ACTOR_ARG(port));
                    auto res = interfaces::native_address(host, std::move(preferred));
                    if (!res) {
                        ACTOR_LOG_DEBUG("no such host");
                        return make_error(sec::cannot_connect_to_node, "no such host", host, port);
                    }
                    auto proto = res->second;
                    ACTOR_ASSERT(proto == ipv4 || proto == ipv6);
                    int socktype = SOCK_STREAM;
#ifdef SOCK_CLOEXEC
                    socktype |= SOCK_CLOEXEC;
#endif
                    CALL_CFUN(fd, detail::cc_valid_socket, "socket",
                              socket(proto == ipv4 ? AF_INET : AF_INET6, socktype, 0));
                    child_process_inherit(fd, false);
                    detail::socket_guard sguard(fd);
                    if (proto == ipv6) {
                        if (ip_connect<AF_INET6>(fd, res->first, port)) {
                            ACTOR_LOG_INFO("successfully connected to (IPv6):" << ACTOR_ARG(host) << ACTOR_ARG(port));
                            return sguard.release();
                        }
                        sguard.close();
                        // IPv4 fallback
                        return new_tcp_connection(host, port, ipv4);
                    }
                    if (!ip_connect<AF_INET>(fd, res->first, port)) {
                        ACTOR_LOG_WARNING("could not connect to:" << ACTOR_ARG(host) << ACTOR_ARG(port));
                        return make_error(sec::cannot_connect_to_node, "ip_connect failed", host, port);
                    }
                    ACTOR_LOG_INFO("successfully connected to (IPv4):" << ACTOR_ARG(host) << ACTOR_ARG(port));
                    return sguard.release();
                }

                template<class SockAddrType>
                expected<void> read_port(native_socket fd, SockAddrType &sa) {
                    socket_size_type len = sizeof(SockAddrType);
                    CALL_CFUN(res, detail::cc_zero, "getsockname",
                              getsockname(fd, reinterpret_cast<sockaddr *>(&sa), &len));
                    return unit;
                }

                expected<void> set_inaddr_any(native_socket, sockaddr_in &sa) {
                    sa.sin_addr.s_addr = INADDR_ANY;
                    return unit;
                }

                expected<void> set_inaddr_any(native_socket fd, sockaddr_in6 &sa) {
                    sa.sin6_addr = in6addr_any;
                    // also accept ipv4 requests on this socket
                    int off = 0;
                    CALL_CFUN(res, detail::cc_zero, "setsockopt",
                              setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<setsockopt_ptr>(&off),
                                         static_cast<socket_size_type>(sizeof(off))));
                    return unit;
                }

                template<int Family, int SockType = SOCK_STREAM>
                expected<native_socket> new_ip_acceptor_impl(uint16_t port, const char *addr, bool reuse_addr,
                                                             bool any) {
                    static_assert(Family == AF_INET || Family == AF_INET6, "invalid family");
                    ACTOR_LOG_TRACE(ACTOR_ARG(port) << ", addr = " << (addr ? addr : "nullptr"));
                    int socktype = SockType;
#ifdef SOCK_CLOEXEC
                    socktype |= SOCK_CLOEXEC;
#endif
                    CALL_CFUN(fd, detail::cc_valid_socket, "socket", socket(Family, socktype, 0));
                    child_process_inherit(fd, false);
                    // sguard closes the socket in case of exception
                    detail::socket_guard sguard {fd};
                    if (reuse_addr) {
                        int on = 1;
                        CALL_CFUN(tmp1, detail::cc_zero, "setsockopt",
                                  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<setsockopt_ptr>(&on),
                                             static_cast<socket_size_type>(sizeof(on))));
                    }
                    using sockaddr_type = typename std::conditional<Family == AF_INET, sockaddr_in, sockaddr_in6>::type;
                    sockaddr_type sa;
                    memset(&sa, 0, sizeof(sockaddr_type));
                    family_of(sa) = Family;
                    if (any)
                        set_inaddr_any(fd, sa);
                    CALL_CFUN(tmp, detail::cc_one, "inet_pton", inet_pton(Family, addr, &addr_of(sa)));
                    port_of(sa) = htons(port);
                    CALL_CFUN(res, detail::cc_zero, "bind",
                              bind(fd, reinterpret_cast<sockaddr *>(&sa), static_cast<socket_size_type>(sizeof(sa))));
                    return sguard.release();
                }

                expected<native_socket> new_tcp_acceptor_impl(uint16_t port, const char *addr, bool reuse_addr) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(port) << ", addr = " << (addr ? addr : "nullptr"));
                    auto addrs = interfaces::server_address(port, addr);
                    auto addr_str = std::string {addr == nullptr ? "" : addr};
                    if (addrs.empty())
                        return make_error(sec::cannot_open_port, "No local interface available", addr_str);
                    bool any = addr_str.empty() || addr_str == "::" || addr_str == "0.0.0.0";
                    auto fd = invalid_native_socket;
                    for (auto &elem : addrs) {
                        auto hostname = elem.first.c_str();
                        auto p = elem.second == ipv4 ? new_ip_acceptor_impl<AF_INET>(port, hostname, reuse_addr, any) :
                                                       new_ip_acceptor_impl<AF_INET6>(port, hostname, reuse_addr, any);
                        if (!p) {
                            ACTOR_LOG_DEBUG(p.error());
                            continue;
                        }
                        fd = *p;
                        break;
                    }
                    if (fd == invalid_native_socket) {
                        ACTOR_LOG_WARNING("could not open tcp socket on:" << ACTOR_ARG(port) << ACTOR_ARG(addr_str));
                        return make_error(sec::cannot_open_port, "tcp socket creation failed", port, addr_str);
                    }
                    detail::socket_guard sguard {fd};
                    CALL_CFUN(tmp2, detail::cc_zero, "listen", listen(fd, SOMAXCONN));
                    // ok, no errors so far
                    ACTOR_LOG_DEBUG(ACTOR_ARG(fd));
                    return sguard.release();
                }

                expected<std::pair<native_socket, ip_endpoint>>
                    new_remote_udp_endpoint_impl(const std::string &host, uint16_t port,
                                                 optional<protocol::network> preferred) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(host) << ACTOR_ARG(port) << ACTOR_ARG(preferred));
                    auto lep = new_local_udp_endpoint_impl(0, nullptr, false, preferred);
                    if (!lep)
                        return std::move(lep.error());
                    detail::socket_guard sguard {(*lep).first};
                    std::pair<native_socket, ip_endpoint> info;
                    memset(std::get<1>(info).address(), 0, sizeof(sockaddr_storage));
                    if (!interfaces::get_endpoint(host, port, std::get<1>(info), (*lep).second))
                        return make_error(sec::cannot_connect_to_node, "no such host", host, port);
                    get<0>(info) = sguard.release();
                    return info;
                }

                expected<std::pair<native_socket, protocol::network>>
                    new_local_udp_endpoint_impl(uint16_t port, const char *addr, bool reuse,
                                                optional<protocol::network> preferred) {
                    ACTOR_LOG_TRACE(ACTOR_ARG(port) << ", addr = " << (addr ? addr : "nullptr"));
                    auto addrs = interfaces::server_address(port, addr, preferred);
                    auto addr_str = std::string {addr == nullptr ? "" : addr};
                    if (addrs.empty())
                        return make_error(sec::cannot_open_port, "No local interface available", addr_str);
                    bool any = addr_str.empty() || addr_str == "::" || addr_str == "0.0.0.0";
                    auto fd = invalid_native_socket;
                    protocol::network proto;
                    for (auto &elem : addrs) {
                        auto host = elem.first.c_str();
                        auto p = elem.second == ipv4 ?
                                     new_ip_acceptor_impl<AF_INET, SOCK_DGRAM>(port, host, reuse, any) :
                                     new_ip_acceptor_impl<AF_INET6, SOCK_DGRAM>(port, host, reuse, any);
                        if (!p) {
                            ACTOR_LOG_DEBUG(p.error());
                            continue;
                        }
                        fd = *p;
                        proto = elem.second;
                        break;
                    }
                    if (fd == invalid_native_socket) {
                        ACTOR_LOG_WARNING("could not open udp socket on:" << ACTOR_ARG(port) << ACTOR_ARG(addr_str));
                        return make_error(sec::cannot_open_port, "udp socket creation failed", port, addr_str);
                    }
                    ACTOR_LOG_DEBUG(ACTOR_ARG(fd));
                    return std::make_pair(fd, proto);
                }

            }    // namespace network
        }        // namespace io
    }            // namespace actor
}    // namespace nil
