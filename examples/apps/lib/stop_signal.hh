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

#include <nil/actor/core/sharded.hh>
#include <nil/actor/core/reactor.hh>
#include <nil/actor/core/condition-variable.hh>

/// Actor apps lib namespace

namespace seastar_apps_lib {

    /// \brief Futurized SIGINT/SIGTERM signals handler class
    ///
    /// Actor-style helper class that allows easy waiting for SIGINT/SIGTERM signals
    /// from your app.
    ///
    /// Example:
    /// \code
    /// #include <seastar/apps/lib/stop_signal.hh>
    /// ...
    /// int main() {
    /// ...
    /// nil::actor::thread th([] {
    ///    seastar_apps_lib::stop_signal stop_signal;
    ///    <some code>
    ///    stop_signal.wait().get();  // this will wait till we receive SIGINT or SIGTERM signal
    /// });
    /// \endcode
    class stop_signal {
        bool _caught = false;
        nil::actor::condition_variable _cond;

    private:
        void signaled() {
            if (_caught) {
                return;
            }
            _caught = true;
            _cond.broadcast();
        }

    public:
        stop_signal() {
            nil::actor::engine().handle_signal(SIGINT, [this] { signaled(); });
            nil::actor::engine().handle_signal(SIGTERM, [this] { signaled(); });
        }
        ~stop_signal() {
            // There's no way to unregister a handler yet, so register a no-op handler instead.
            nil::actor::engine().handle_signal(SIGINT, [] {});
            nil::actor::engine().handle_signal(SIGTERM, [] {});
        }
        nil::actor::future<> wait() {
            return _cond.wait([this] { return _caught; });
        }
        bool stopping() const {
            return _caught;
        }
    };
}    // namespace seastar_apps_lib
