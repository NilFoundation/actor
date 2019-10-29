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

#pragma once

#include <iosfwd>

namespace nil {
    namespace mtl {

        /// Terminal color and font face options.
        enum class term {
            /// Resets the color to the default color and the font weight to normal.
            reset,
            /// Like `reset` but also prints a newline.
            reset_endl,
            /// Sets the terminal color to black.
            black,
            /// Sets the terminal color to red.
            red,
            /// Sets the terminal color to green.
            green,
            /// Sets the terminal color to yellow.
            yellow,
            /// Sets the terminal color to blue.
            blue,
            /// Sets the terminal color to magenta.
            magenta,
            /// Sets the terminal color to cyan.
            cyan,
            /// Sets the terminal color to white.
            white,
            /// Sets the terminal color to black and the font weight to bold.
            bold_black,
            /// Sets the terminal color to red and the font weight to bold.
            bold_red,
            /// Sets the terminal color to green and the font weight to bold.
            bold_green,
            /// Sets the terminal color to yellow and the font weight to bold.
            bold_yellow,
            /// Sets the terminal color to blue and the font weight to bold.
            bold_blue,
            /// Sets the terminal color to magenta and the font weight to bold.
            bold_magenta,
            /// Sets the terminal color to cyan and the font weight to bold.
            bold_cyan,
            /// Sets the terminal color to white and the font weight to bold.
            bold_white
        };

        std::ostream &operator<<(std::ostream &out, term x);

    }    // namespace mtl
}    // namespace nil
