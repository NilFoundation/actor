//---------------------------------------------------------------------------//
// Copyright (c) 2011-2018 Dominik Charousset
// Copyright (c) 2018-2019 Nil Foundation AG
// Copyright (c) 2018-2019 Mikhail Komarov <nemo@nil.foundation>
//
// Distributed under the terms and conditions of the BSD 3-Clause License or
// (at your option) under the terms and conditions of the Boost Software
// License 1.0. See accompanying files LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt.
//---------------------------------------------------------------------------//

#pragma once

namespace boost {
    template<typename T>
    class intrusive_ptr;
}

namespace nil {
    namespace mtl {

        // -- templates from the parent namespace necessary for defining aliases -------

        template<typename T>
        using intrusive_ptr = boost::intrusive_ptr<T>;

        namespace io {

            // -- variadic templates -------------------------------------------------------

            template<class... Sigs>
            class typed_broker;

            // -- classes ------------------------------------------------------------------

            class scribe;
            class broker;
            class doorman;
            class middleman;
            class basp_broker;
            class receive_policy;
            class abstract_broker;
            class datagram_servant;

            // -- aliases ------------------------------------------------------------------

            using scribe_ptr = intrusive_ptr<scribe>;
            using doorman_ptr = intrusive_ptr<doorman>;
            using datagram_servant_ptr = intrusive_ptr<datagram_servant>;

            // -- nested namespaces --------------------------------------------------------

            namespace network {

                class multiplexer;
                class default_multiplexer;

            }    // namespace network

        }    // namespace io
    }        // namespace mtl
}    // namespace nil
