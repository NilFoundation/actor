//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt for Boost License or
// http://opensource.org/licenses/BSD-3-Clause for BSD 3-Clause License
//---------------------------------------------------------------------------//

#include <nil/actor/openssl/session.hpp>

ACTOR_PUSH_WARNINGS
#include <openssl/err.h>
ACTOR_POP_WARNINGS

#include <nil/actor/spawner_config.hpp>

#include <nil/actor/io/network/default_multiplexer.hpp>

#include <nil/actor/openssl/manager.hpp>

// On Linux we need to block SIGPIPE whenever we access OpenSSL functions.
// Unfortunately there's no sane way to configure OpenSSL properly.
#ifdef BOOST_OS_LINUX_AVAILABLE

#include <nil/actor/detail/scope_guard.hpp>
#include <signal.h>

#define ACTOR_BLOCK_SIGPIPE()                                             \
    sigset_t sigpipe_mask;                                              \
    sigemptyset(&sigpipe_mask);                                         \
    sigaddset(&sigpipe_mask, SIGPIPE);                                  \
    sigset_t saved_mask;                                                \
    if (pthread_sigmask(SIG_BLOCK, &sigpipe_mask, &saved_mask) == -1) { \
        perror("pthread_sigmask");                                      \
        exit(1);                                                        \
    }                                                                   \
    auto sigpipe_restore_guard = ::actor::detail::make_scope_guard([&] {  \
        struct timespec zerotime = {0};                                 \
        sigtimedwait(&sigpipe_mask, 0, &zerotime);                      \
        if (pthread_sigmask(SIG_SETMASK, &saved_mask, 0) == -1) {       \
            perror("pthread_sigmask");                                  \
            exit(1);                                                    \
        }                                                               \
    })

#else

#define ACTOR_BLOCK_SIGPIPE() static_cast<void>(0)

#endif    // BOOST_OS_LINUX_AVAILABLE

namespace nil {
    namespace actor {
        namespace openssl {

            namespace {

                int pem_passwd_cb(char *buf, int size, int, void *ptr) {
                    auto passphrase = reinterpret_cast<session *>(ptr)->openssl_passphrase();
                    strncpy(buf, passphrase, static_cast<size_t>(size));
                    buf[size - 1] = '\0';
                    return static_cast<int>(strlen(buf));
                }

            }    // namespace

            session::session(spawner &sys) :
                sys_(sys), ctx_(nullptr), ssl_(nullptr), connecting_(false), accepting_(false) {
                // nop
            }

            bool session::init() {
                ACTOR_LOG_TRACE("");
                ctx_ = create_ssl_context();
                ssl_ = SSL_new(ctx_);
                if (ssl_ == nullptr) {
                    ACTOR_LOG_ERROR("cannot create SSL session");
                    return false;
                }
                return true;
            }

            session::~session() {
                SSL_free(ssl_);
                SSL_CTX_free(ctx_);
            }

            rw_state session::do_some(int (*f)(SSL *, void *, int), size_t &result, void *buf, size_t len,
                                      const char *debug_name) {
                ACTOR_BLOCK_SIGPIPE();
                auto check_ssl_res = [&](int res) -> rw_state {
                    result = 0;
                    switch (SSL_get_error(ssl_, res)) {
                        default:
                            ACTOR_LOG_INFO("SSL error:" << get_ssl_error());
                            return rw_state::failure;
                        case SSL_ERROR_WANT_READ:
                            ACTOR_LOG_DEBUG("SSL_ERROR_WANT_READ reported");
                            // Report success to poll on this socket.
                            if (len == 0 && strcmp(debug_name, "write_some") == 0)
                                return rw_state::indeterminate;
                            return rw_state::success;
                        case SSL_ERROR_WANT_WRITE:
                            ACTOR_LOG_DEBUG("SSL_ERROR_WANT_WRITE reported");
                            // Report success to poll on this socket.
                            return rw_state::success;
                    }
                };
                ACTOR_LOG_TRACE(ACTOR_ARG(len) << ACTOR_ARG(debug_name));
                ACTOR_IGNORE_UNUSED(debug_name);
                if (connecting_) {
                    ACTOR_LOG_DEBUG(debug_name << ": connecting");
                    auto res = SSL_connect(ssl_);
                    if (res == 1) {
                        ACTOR_LOG_DEBUG("SSL connection established");
                        connecting_ = false;
                    } else {
                        result = 0;
                        return check_ssl_res(res);
                    }
                }
                if (accepting_) {
                    ACTOR_LOG_DEBUG(debug_name << ": accepting");
                    auto res = SSL_accept(ssl_);
                    if (res == 1) {
                        ACTOR_LOG_DEBUG("SSL connection accepted");
                        accepting_ = false;
                    } else {
                        result = 0;
                        return check_ssl_res(res);
                    }
                }
                ACTOR_LOG_DEBUG(debug_name << ": calling SSL_write or SSL_read");
                if (len == 0) {
                    result = 0;
                    return rw_state::indeterminate;
                }
                auto ret = f(ssl_, buf, static_cast<int>(len));
                if (ret > 0) {
                    result = static_cast<size_t>(ret);
                    return rw_state::success;
                }
                result = 0;
                return handle_ssl_result(ret) ? rw_state::success : rw_state::failure;
            }

            rw_state session::read_some(size_t &result, native_socket, void *buf, size_t len) {
                ACTOR_LOG_TRACE(ACTOR_ARG(len));
                return do_some(SSL_read, result, buf, len, "read_some");
            }

            rw_state session::write_some(size_t &result, native_socket, const void *buf, size_t len) {
                ACTOR_LOG_TRACE(ACTOR_ARG(len));
                auto wr_fun = [](SSL *sptr, void *vptr, int ptr_size) { return SSL_write(sptr, vptr, ptr_size); };
                return do_some(wr_fun, result, const_cast<void *>(buf), len, "write_some");
            }

            bool session::try_connect(native_socket fd) {
                ACTOR_LOG_TRACE(ACTOR_ARG(fd));
                ACTOR_BLOCK_SIGPIPE();
                SSL_set_fd(ssl_, fd);
                SSL_set_connect_state(ssl_);
                auto ret = SSL_connect(ssl_);
                if (ret == 1)
                    return true;
                connecting_ = true;
                return handle_ssl_result(ret);
            }

            bool session::try_accept(native_socket fd) {
                ACTOR_LOG_TRACE(ACTOR_ARG(fd));
                ACTOR_BLOCK_SIGPIPE();
                SSL_set_fd(ssl_, fd);
                SSL_set_accept_state(ssl_);
                auto ret = SSL_accept(ssl_);
                if (ret == 1)
                    return true;
                accepting_ = true;
                return handle_ssl_result(ret);
            }

            bool session::must_read_more(native_socket, size_t threshold) {
                return static_cast<size_t>(SSL_pending(ssl_)) >= threshold;
            }

            const char *session::openssl_passphrase() {
                return openssl_passphrase_.c_str();
            }

            SSL_CTX *session::create_ssl_context() {
                ACTOR_BLOCK_SIGPIPE();
#ifdef ACTOR_SSL_HAS_NON_VERSIONED_TLS_FUN
                auto ctx = SSL_CTX_new(TLS_method());
#else
                auto ctx = SSL_CTX_new(TLSv1_2_method());
#endif
                if (!ctx)
                    ACTOR_RAISE_ERROR("cannot create OpenSSL context");
                if (sys_.openssl_manager().authentication_enabled()) {
                    // Require valid certificates on both sides.
                    auto &cfg = sys_.config();
                    if (cfg.openssl_certificate.size() > 0 &&
                        SSL_CTX_use_certificate_chain_file(ctx, cfg.openssl_certificate.c_str()) != 1)
                        ACTOR_RAISE_ERROR("cannot load certificate");
                    if (cfg.openssl_passphrase.size() > 0) {
                        openssl_passphrase_ = cfg.openssl_passphrase;
                        SSL_CTX_set_default_passwd_cb(ctx, pem_passwd_cb);
                        SSL_CTX_set_default_passwd_cb_userdata(ctx, this);
                    }
                    if (cfg.openssl_key.size() > 0 &&
                        SSL_CTX_use_PrivateKey_file(ctx, cfg.openssl_key.c_str(), SSL_FILETYPE_PEM) != 1)
                        ACTOR_RAISE_ERROR("cannot load private key");
                    auto cafile = (cfg.openssl_cafile.size() > 0 ? cfg.openssl_cafile.c_str() : nullptr);
                    auto capath = (cfg.openssl_capath.size() > 0 ? cfg.openssl_capath.c_str() : nullptr);
                    if (cafile || capath) {
                        if (SSL_CTX_load_verify_locations(ctx, cafile, capath) != 1)
                            ACTOR_RAISE_ERROR("cannot load trusted CA certificates");
                    }
                    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, nullptr);
                    if (SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5") != 1)
                        ACTOR_RAISE_ERROR("cannot set cipher list");
                } else {
                    // No authentication.
                    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, nullptr);
#if defined(ACTOR_SSL_HAS_ECDH_AUTO) && (OPENSSL_VERSION_NUMBER < 0x10100000L)
                    SSL_CTX_set_ecdh_auto(ctx, 1);
#else
                    auto ecdh = EC_KEY_new_by_curve_name(NID_secp384r1);
                    if (!ecdh)
                        ACTOR_RAISE_ERROR("cannot get ECDH curve");
                    ACTOR_PUSH_WARNINGS
                    SSL_CTX_set_tmp_ecdh(ctx, ecdh);
                    EC_KEY_free(ecdh);
                    ACTOR_POP_WARNINGS
#endif
#ifdef ACTOR_SSL_HAS_SECURITY_LEVEL
                    const char *cipher = "AECDH-AES256-SHA@SECLEVEL=0";
#else
                    const char *cipher = "AECDH-AES256-SHA";
#endif
                    if (SSL_CTX_set_cipher_list(ctx, cipher) != 1)
                        ACTOR_RAISE_ERROR("cannot set anonymous cipher");
                }
                return ctx;
            }

            std::string session::get_ssl_error() {
                std::string msg = "";
                while (auto err = ERR_get_error()) {
                    if (msg.size() > 0)
                        msg += " ";
                    char buf[256];
                    ERR_error_string_n(err, buf, sizeof(buf));
                    msg += buf;
                }
                return msg;
            }

            bool session::handle_ssl_result(int ret) {
                auto err = SSL_get_error(ssl_, ret);
                switch (err) {
                    case SSL_ERROR_WANT_READ:
                        ACTOR_LOG_DEBUG("Nonblocking call to SSL returned want_read");
                        return true;
                    case SSL_ERROR_WANT_WRITE:
                        ACTOR_LOG_DEBUG("Nonblocking call to SSL returned want_write");
                        return true;
                    case SSL_ERROR_ZERO_RETURN:    // Regular remote connection shutdown.
                    case SSL_ERROR_SYSCALL:        // Socket connection closed.
                        return false;
                    default:    // Other error
                        ACTOR_LOG_INFO("SSL call failed:" << get_ssl_error());
                        return false;
                }
            }

            session_ptr make_session(spawner &sys, native_socket fd, bool from_accepted_socket) {
                session_ptr ptr {new session(sys)};
                if (!ptr->init())
                    return nullptr;
                if (from_accepted_socket) {
                    if (!ptr->try_accept(fd))
                        return nullptr;
                } else {
                    if (!ptr->try_connect(fd))
                        return nullptr;
                }
                return ptr;
            }

        }    // namespace openssl
    }        // namespace actor
}