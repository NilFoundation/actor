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

#include <nil/mtl/detail/stringification_inspector.hpp>

#include <algorithm>
#include <ctime>

#include <nil/mtl/atom.hpp>

namespace {

    void escape(std::string &result, char c) {
        switch (c) {
            default:
                result += c;
                break;
            case '\n':
                result += R"(\n)";
                break;
            case '\t':
                result += R"(\t)";
                break;
            case '\\':
                result += R"(\\)";
                break;
            case '"':
                result += R"(\")";
                break;
        }
    }

}    // namespace

namespace nil {
    namespace mtl {
        namespace detail {

            void stringification_inspector::sep() {
                if (!result_.empty())
                    switch (result_.back()) {
                        case '(':
                        case '[':
                        case '{':
                        case ' ':    // only at back if we've printed ", " before
                            break;
                        default:
                            result_ += ", ";
                    }
            }

            void stringification_inspector::consume(atom_value x) {
                result_ += '\'';
                result_ += to_string(x);
                result_ += '\'';
            }

            void stringification_inspector::consume(string_view str) {
                if (str.empty()) {
                    result_ += R"("")";
                    return;
                }
                if (str[0] == '"') {
                    // Assume an already escaped string.
                    result_.insert(result_.end(), str.begin(), str.end());
                    return;
                }
                // Escape string.
                result_ += '"';
                for (char c : str)
                    escape(result_, c);
                result_ += '"';
            }

            void stringification_inspector::consume(timespan x) {
                auto ns = x.count();
                if (ns % 1000 > 0) {
                    consume(ns);
                    result_ += "ns";
                    return;
                }
                auto us = ns / 1000;
                if (us % 1000 > 0) {
                    consume(us);
                    result_ += "us";
                    return;
                }
                auto ms = us / 1000;
                if (ms % 1000 > 0) {
                    consume(ms);
                    result_ += "ms";
                    return;
                }
                auto s = ms / 1000;
                if (s % 60 > 0) {
                    consume(s);
                    result_ += "s";
                    return;
                }
                auto min = s / 60;
                if (min % 60 > 0) {
                    consume(min);
                    result_ += "min";
                    return;
                }
                auto h = min / 60;
                if (min % 24 > 0) {
                    consume(h);
                    result_ += "h";
                    return;
                }
                auto d = h / 24;
                consume(d);
                result_ += "d";
            }

            void stringification_inspector::consume(timestamp x) {
                char buf[64];
                auto y = std::chrono::time_point_cast<timestamp::clock::duration>(x);
                auto z = timestamp::clock::to_time_t(y);
                strftime(buf, sizeof(buf), "%FT%T", std::localtime(&z));
                result_ += buf;
                // time_t has no milliseconds, so we need to insert them manually.
                auto ms = (x.time_since_epoch().count() / 1000000) % 1000;
                result_ += '.';
                auto frac = std::to_string(ms);
                if (frac.size() < 3)
                    frac.insert(0, 3 - frac.size(), '0');
                result_ += frac;
            }

            void stringification_inspector::consume(bool x) {
                result_ += x ? "true" : "false";
            }

            void stringification_inspector::consume(const void *ptr) {
                result_ += "0x";
                consume(reinterpret_cast<intptr_t>(ptr));
            }

            void stringification_inspector::consume(const char *cstr) {
                if (cstr == nullptr) {
                    result_ += "<null>";
                    return;
                }
                if (cstr[0] == '\0') {
                    result_ += R"("")";
                    return;
                }
                if (cstr[0] == '"') {
                    // Assume an already escaped string.
                    result_ += cstr;
                    return;
                }
                // Escape string.
                result_ += '"';
                for (char c = *cstr++; c != '\0'; c = *cstr++)
                    escape(result_, c);
                result_ += '"';
            }

            void stringification_inspector::consume(const std::vector<bool> &xs) {
                result_ += '[';
                for (bool x : xs) {
                    sep();
                    consume(x);
                }
                result_ += ']';
            }

            void stringification_inspector::consume_int(int64_t x) {
                if (x >= 0)
                    return consume_int(static_cast<uint64_t>(x));
                result_ += '-';
                auto begin = result_.size();
                result_ += -(x % 10) + '0';
                x /= 10;
                while (x != 0) {
                    result_ += -(x % 10) + '0';
                    x /= 10;
                }
                std::reverse(result_.begin() + begin, result_.end());
            }

            void stringification_inspector::consume_int(uint64_t x) {
                auto begin = result_.size();
                result_ += (x % 10) + '0';
                x /= 10;
                while (x != 0) {
                    result_ += (x % 10) + '0';
                    x /= 10;
                }
                std::reverse(result_.begin() + begin, result_.end());
            }

        }    // namespace detail
    }        // namespace mtl
}