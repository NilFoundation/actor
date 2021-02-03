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

// Demonstration of nil::actor::sharded_parameter

#include <nil/actor/core/sharded.hh>
#include <nil/actor/core/app-template.hh>
#include <nil/actor/core/thread.hh>
#include <nil/actor/detail/defer.hh>
#include <cassert>

// This is some service that we wish to run on all shards.
class service_one {
    int _capacity = 7;

public:
    // Pretend that this int is some important resource.
    int get_capacity() const {
        return _capacity;
    }
};

// Another service that we run on all shards, that depends on service_one.
class service_two {
    int _resource_allocation;

public:
    service_two(service_one &s1, int resource_allocation) : _resource_allocation(resource_allocation) {
    }
    int get_resource_allocation() const {
        return _resource_allocation;
    }
};

int main(int ac, char **av) {
    nil::actor::app_template app;
    return app.run(ac, av, [&] {
        // sharded<> setup code is typically run in a nil::actor::thread
        return nil::actor::async([&] {
            // Launch service_one
            nil::actor::sharded<service_one> s1;
            s1.start().get();
            auto stop_s1 = nil::actor::defer([&] { s1.stop().get(); });

            auto calculate_half_capacity = [](service_one &s1) { return s1.get_capacity() / 2; };

            // Launch service_two, passing it per-shard dependencies from s1
            nil::actor::sharded<service_two> s2;
            // Start s2, passing two parameters to service_two's constructor
            s2.start(
                  // Each service_two instance will get a reference to a service_one instance on the same shard
                  std::ref(s1),
                  // This calculation will be performed on each shard
                  nil::actor::sharded_parameter(calculate_half_capacity, std::ref(s1)))
                .get();
            nil::actor::defer([&] { s2.stop().get(); });

            s2.invoke_on_all([](service_two &s2) { assert(s2.get_resource_allocation() == 3); }).get();
        });
    });
}
