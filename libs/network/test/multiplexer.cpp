//---------------------------------------------------------------------------//
// Copyright (c) 2011-2019 Dominik Charousset
// Copyright (c) 2018-2020 Nil Foundation AG
// Copyright (c) 2018-2020 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#define BOOST_TEST_MODULE multiplexer

#include <nil/mtl/network/multiplexer.hpp>

#include <nil/mtl/test/host_fixture.hpp>
#include <nil/mtl/test/dsl.hpp>

#include <new>
#include <tuple>
#include <vector>

#include <nil/mtl/byte.hpp>
#include <nil/mtl/network/socket_manager.hpp>
#include <nil/mtl/network/stream_socket.hpp>
#include <nil/mtl/span.hpp>

using namespace nil::mtl;
using namespace nil::mtl::network;

namespace {

    class dummy_manager : public socket_manager {
    public:
        dummy_manager(size_t &manager_count, stream_socket handle, multiplexer_ptr parent) :
            socket_manager(handle, std::move(parent)), count_(manager_count), rd_buf_pos_(0) {
            ++count_;
            rd_buf_.resize(1024);
        }

        ~dummy_manager() {
            --count_;
        }

        stream_socket handle() const noexcept {
            return socket_cast<stream_socket>(handle_);
        }

        bool handle_read_event() override {
            if (read_capacity() < 1024)
                rd_buf_.resize(rd_buf_.size() + 2048);
            auto res = read(handle(), make_span(read_position_begin(), read_capacity()));
            if (auto num_bytes = get_if<size_t>(&res)) {
                MTL_ASSERT(*num_bytes > 0);
                rd_buf_pos_ += *num_bytes;
                return true;
            }
            return get<sec>(res) == sec::unavailable_or_would_block;
        }

        bool handle_write_event() override {
            if (wr_buf_.size() == 0)
                return false;
            auto res = write(handle(), wr_buf_);
            if (auto num_bytes = get_if<size_t>(&res)) {
                MTL_ASSERT(*num_bytes > 0);
                wr_buf_.erase(wr_buf_.begin(), wr_buf_.begin() + *num_bytes);
                return wr_buf_.size() > 0;
            }
            return get<sec>(res) == sec::unavailable_or_would_block;
        }

        void handle_error(sec code) override {
            BOOST_FAIL("handle_error called with code " << code);
        }

        void send(string_view x) {
            auto x_bytes = as_bytes(make_span(x));
            wr_buf_.insert(wr_buf_.end(), x_bytes.begin(), x_bytes.end());
        }

        std::string receive() {
            std::string result(reinterpret_cast<char *>(rd_buf_.data()), rd_buf_pos_);
            rd_buf_pos_ = 0;
            return result;
        }

    private:
        byte *read_position_begin() {
            return rd_buf_.data() + rd_buf_pos_;
        }

        byte *read_position_end() {
            return rd_buf_.data() + rd_buf_.size();
        }

        size_t read_capacity() const {
            return rd_buf_.size() - rd_buf_pos_;
        }

        size_t &count_;

        size_t rd_buf_pos_;

        std::vector<byte> wr_buf_;

        std::vector<byte> rd_buf_;
    };

    using dummy_manager_ptr = intrusive_ptr<dummy_manager>;

    struct fixture : host_fixture {
        fixture() : manager_count(0), mpx(std::make_shared<multiplexer>()) {
            mpx->set_thread_id();
        }

        ~fixture() {
            mpx.reset();
            BOOST_REQUIRE_EQUAL(manager_count, 0u);
        }

        void exhaust() {
            while (mpx->poll_once(false))
                ;    // Repeat.
        }

        size_t manager_count;

        multiplexer_ptr mpx;
    };

}    // namespace

BOOST_FIXTURE_TEST_SUITE(multiplexer_tests, fixture)

BOOST_AUTO_TEST_CASE(default construction) {
    BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 0u);
}

BOOST_AUTO_TEST_CASE(init) {
    BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 0u);
    BOOST_REQUIRE_EQUAL(mpx->init(), none);
    BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 1u);
    mpx->close_pipe();
    exhaust();
    BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 0u);
    // Calling run must have no effect now.
    mpx->run();
}

BOOST_AUTO_TEST_CASE(send and receive) {
    BOOST_REQUIRE_EQUAL(mpx->init(), none);
    auto sockets = unbox(make_stream_socket_pair());
    auto alice = make_counted<dummy_manager>(manager_count, sockets.first, mpx);
    auto bob = make_counted<dummy_manager>(manager_count, sockets.second, mpx);
    alice->register_reading();
    bob->register_reading();
    BOOST_CHECK_EQUAL(mpx->num_socket_managers(), 3u);
    alice->send("hello bob");
    alice->register_writing();
    exhaust();
    BOOST_CHECK_EQUAL(bob->receive(), "hello bob");
}

BOOST_AUTO_TEST_CASE(shutdown) {
    std::mutex m;
    std::condition_variable cv;
    bool thread_id_set = false;
    auto run_mpx = [&] {
        std::unique_lock<std::mutex> lk(m);
        mpx->set_thread_id();
        thread_id_set = true;
        lk.unlock();
        cv.notify_one();
        mpx->run();
    };
    BOOST_REQUIRE_EQUAL(mpx->init(), none);
    auto sockets = unbox(make_stream_socket_pair());
    auto alice = make_counted<dummy_manager>(manager_count, sockets.first, mpx);
    auto bob = make_counted<dummy_manager>(manager_count, sockets.second, mpx);
    alice->register_reading();
    bob->register_reading();
    BOOST_REQUIRE_EQUAL(mpx->num_socket_managers(), 3u);
    std::thread mpx_thread {run_mpx};
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk, [&] { return thread_id_set; });
    mpx->shutdown();
    mpx_thread.join();
    BOOST_REQUIRE_EQUAL(mpx->num_socket_managers(), 0u);
}

BOOST_AUTO_TEST_SUITE_END()
