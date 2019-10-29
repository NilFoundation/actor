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

#include <nil/mtl/uri.hpp>

#include <nil/mtl/deserializer.hpp>
#include <nil/mtl/detail/append_percent_encoded.hpp>
#include <nil/mtl/detail/fnv_hash.hpp>
#include <nil/mtl/detail/overload.hpp>
#include <nil/mtl/detail/parse.hpp>
#include <nil/mtl/detail/parser/read_uri.hpp>
#include <nil/mtl/detail/uri_impl.hpp>
#include <nil/mtl/error.hpp>
#include <nil/mtl/make_counted.hpp>
#include <nil/mtl/serializer.hpp>

namespace nil {
    namespace mtl {

        uri::uri() : impl_(&detail::uri_impl::default_instance) {
            // nop
        }

        uri::uri(impl_ptr ptr) : impl_(std::move(ptr)) {
            MTL_ASSERT(impl_ != nullptr);
        }

        bool uri::empty() const noexcept {
            return str().empty();
        }

        string_view uri::str() const noexcept {
            return impl_->str;
        }

        string_view uri::scheme() const noexcept {
            return impl_->scheme;
        }

        const uri::authority_type &uri::authority() const noexcept {
            return impl_->authority;
        }

        string_view uri::path() const noexcept {
            return impl_->path;
        }

        const uri::query_map &uri::query() const noexcept {
            return impl_->query;
        }

        string_view uri::fragment() const noexcept {
            return impl_->fragment;
        }

        size_t uri::hash_code() const noexcept {
            return detail::fnv_hash(str());
        }

        // -- comparison ---------------------------------------------------------------

        int uri::compare(const uri &other) const noexcept {
            return str().compare(other.str());
        }

        int uri::compare(string_view x) const noexcept {
            return string_view {str()}.compare(x);
        }

        // -- friend functions ---------------------------------------------------------

        error inspect(nil::mtl::serializer &dst, uri &x) {
            return inspect(dst, const_cast<detail::uri_impl &>(*x.impl_));
        }

        error inspect(nil::mtl::deserializer &src, uri &x) {
            auto impl = make_counted<detail::uri_impl>();
            auto err = inspect(src, *impl);
            if (err == none)
                x = uri {std::move(impl)};
            return err;
        }

        // -- related free functions ---------------------------------------------------

        std::string to_string(const uri &x) {
            auto x_str = x.str();
            std::string result {x_str.begin(), x_str.end()};
            return result;
        }

        std::string to_string(const uri::authority_type &x) {
            std::string str;
            if (!x.userinfo.empty()) {
                detail::append_percent_encoded(str, x.userinfo);
                str += '@';
            }
            auto f = nil::mtl::detail::make_overload(
                [&](const ip_address &addr) {
                    if (addr.embeds_v4()) {
                        str += to_string(addr);
                    } else {
                        str += '[';
                        str += to_string(addr);
                        str += ']';
                    }
                },
                [&](const std::string &host) { detail::append_percent_encoded(str, host); });
            visit(f, x.host);
            if (x.port != 0) {
                str += ':';
                str += std::to_string(x.port);
            }
            return str;
        }

        error parse(string_view str, uri &dest) {
            detail::parse_state ps {str.begin(), str.end()};
            parse(ps, dest);
            if (ps.code == pec::success)
                return none;
            return make_error(ps.code, static_cast<size_t>(ps.line), static_cast<size_t>(ps.column));
        }

    }    // namespace mtl
}    // namespace nil
