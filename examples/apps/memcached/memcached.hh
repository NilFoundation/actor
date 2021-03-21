//---------------------------------------------------------------------------//
// Copyright (c) 2018-2021 Mikhail Komarov <nemo@nil.foundation>
//
// MIT License
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//---------------------------------------------------------------------------//

#pragma once

#include <nil/actor/core/sstring.hh>

namespace memcache {

    using namespace nil::actor;

    class item;
    class cache;

    class item_key {
    private:
        sstring _key;
        size_t _hash;

    public:
        item_key() = default;
        item_key(item_key &) = default;
        item_key(sstring key) : _key(key), _hash(std::hash<sstring>()(key)) {
        }
        item_key(item_key &&other) : _key(std::move(other._key)), _hash(other._hash) {
            other._hash = 0;
        }
        size_t hash() const {
            return _hash;
        }
        const sstring &key() const {
            return _key;
        }
        bool operator==(const item_key &other) const {
            return other._hash == _hash && other._key == _key;
        }
        void operator=(item_key &&other) {
            _key = std::move(other._key);
            _hash = other._hash;
            other._hash = 0;
        }
    };

}    // namespace memcache

namespace std {

    template<>
    struct hash<memcache::item_key> {
        size_t operator()(const memcache::item_key &key) {
            return key.hash();
        }
    };

} /* namespace std */
