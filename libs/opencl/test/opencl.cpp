#define BOOST_TEST_MODULE opencl_test

#include <boost/test/unit_test.hpp>

#include <vector>
#include <iomanip>
#include <cassert>
#include <iostream>
#include <algorithm>

#include <nil/mtl/all.hpp>
#include <nil/mtl/system_messages.hpp>

#include <nil/mtl/opencl/all.hpp>

using namespace nil::mtl;
using namespace nil::mtl::opencl;

using nil::mtl::detail::tl_at;
using nil::mtl::detail::tl_head;
using nil::mtl::detail::type_list;

namespace {

    using ivec = std::vector<int>;
    using iref = mem_ref<int>;
    using dims = opencl::dim_vec;

    constexpr std::size_t matrix_size = 4;
    constexpr std::size_t array_size = 32;
    constexpr std::size_t problem_size = 1024;

    constexpr const char *kn_matrix = "matrix_square";
    constexpr const char *kn_compiler_flag = "compiler_flag";
    constexpr const char *kn_reduce = "reduce";
    constexpr const char *kn_const = "const_mod";
    constexpr const char *kn_inout = "times_two";
    constexpr const char *kn_scratch = "use_scratch";
    constexpr const char *kn_local = "use_local";
    constexpr const char *kn_order = "test_order";
    constexpr const char *kn_private = "use_private";
    constexpr const char *kn_varying = "varying";

    constexpr const char *compiler_flag = "-D MTL_OPENCL_TEST_FLAG";

    constexpr const char *kernel_source = R"__(
  kernel void matrix_square(global const int* restrict matrix,
                            global       int* restrict output) {
    size_t size = get_global_size(0); // == get_global_size_(1);
    size_t x = get_global_id(0);
    size_t y = get_global_id(1);
    int result = 0;
    for (size_t idx = 0; idx < size; ++idx) {
      result += matrix[idx + y * size] * matrix[x + idx * size];
    }
    output[x + y * size] = result;
  }

// http://developer.amd.com/resources/documentation-articles/
// articles-whitepapers/opencl-optimization-case-study-simple-reductions
  kernel void reduce(global const int* restrict buffer,
                     global       int* restrict result) {
    local int scratch[512];
    int local_index = get_local_id(0);
    scratch[local_index] = buffer[get_global_id(0)];
    barrier(CLK_LOCAL_MEM_FENCE);
    for(int offset = get_local_size(0) / 2; offset > 0; offset = offset / 2) {
      if (local_index < offset) {
        int other = scratch[local_index + offset];
        int mine = scratch[local_index];
        scratch[local_index] = (mine < other) ? mine : other;
      }
      barrier(CLK_LOCAL_MEM_FENCE);
    }
    if (local_index == 0)
      result[get_group_id(0)] = scratch[0];
  }

  kernel void const_mod(constant int* restrict input,
                        global   int* restrict output) {
    size_t idx = get_global_id(0);
    output[idx] = input[0];
  }

  kernel void times_two(global int* restrict values) {
    size_t idx = get_global_id(0);
    values[idx] = values[idx] * 2;
  }

  kernel void use_scratch(global int* restrict values,
                          global int* restrict buf) {
    size_t idx = get_global_id(0);
    buf[idx] = values[idx];
    buf[idx] += values[idx];
    values[idx] = buf[idx];
  }

  inline void prefix_sum(local int* restrict data, size_t len, size_t lids) {
    size_t lid = get_local_id(0);
    size_t inc = 2;
    // reduce
    while (inc <= len) {
      int j = inc >> 1;
      for (int i = (j - 1) + (lid * inc); (i + inc) < len; i += (lids * inc))
        data[i + j] = data[i] + data[i + j];
      inc = inc << 1;
      barrier(CLK_LOCAL_MEM_FENCE);
    }
    // downsweep
    data[len - 1] = 0;
    barrier(CLK_LOCAL_MEM_FENCE);
    while (inc >= 2) {
      int j = inc >> 1;
      for (int i = (j - 1) + (lid * inc); (i + j) <= len; i += (lids * inc)) {
        uint tmp = data[i + j];
        data[i + j] = data[i] + data[i + j];
        data[i] = tmp;
      }
      inc = inc >> 1;
      barrier(CLK_LOCAL_MEM_FENCE);
    }
  }

  kernel void use_local(global int* restrict values,
                        local  int* restrict buf) {
    size_t lid = get_local_id(0);
    size_t gid = get_group_id(0);
    size_t gs = get_local_size(0);
    buf[lid] = values[gid * gs + lid];
    barrier(CLK_LOCAL_MEM_FENCE);
    prefix_sum(buf, gs, gs);
    barrier(CLK_LOCAL_MEM_FENCE);
    values[gid * gs + lid] = buf[lid];
  }

  kernel void test_order(local  int* buf,
                         global int* restrict values) {
    size_t lid = get_local_id(0);
    size_t gid = get_group_id(0);
    size_t gs = get_local_size(0);
    buf[lid] = values[gid * gs + lid];
    barrier(CLK_LOCAL_MEM_FENCE);
    prefix_sum(buf, gs, gs);
    barrier(CLK_LOCAL_MEM_FENCE);
    values[gid * gs + lid] = buf[lid];
  }

  kernel void use_private(global  int* restrict buf,
                          private int  val) {
    buf[get_global_id(0)] += val;
  }

  kernel void varying(global const int* restrict in1,
                      global       int* restrict out1,
                      global const int* restrict in2,
                      global       int* restrict out2) {
    size_t idx = get_global_id(0);
    out1[idx] = in1[idx];
    out2[idx] = in2[idx];
  }
)__";

#ifndef MTL_NO_EXCEPTIONS
    constexpr const char *kernel_source_error = R"__(
  kernel void missing(global int*) {
    size_t semicolon_missing
  }
)__";
#endif    // MTL_NO_EXCEPTIONS

    constexpr const char *kernel_source_compiler_flag = R"__(
  kernel void compiler_flag(global const int* restrict input,
                            global       int* restrict output) {
    size_t x = get_global_id(0);
#   ifdef MTL_OPENCL_TEST_FLAG
    output[x] = input[x];
#   else
    output[x] = 0;
#   endif
  }
)__";

}    // namespace

template<size_t Size>
class square_matrix {
public:
    using value_type = ivec::value_type;
    static constexpr size_t num_elements = Size * Size;

    template<class Inspector>
    friend typename Inspector::result_type inspect(Inspector &f, square_matrix &x) {
        return f(meta::type_name("square_matrix"), x.data_);
    }

    square_matrix(square_matrix &&) = default;

    square_matrix(const square_matrix &) = default;

    square_matrix &operator=(square_matrix &&) = default;

    square_matrix &operator=(const square_matrix &) = default;

    square_matrix() : data_(num_elements) {
        // nop
    }

    explicit square_matrix(ivec d) : data_(move(d)) {
        assert(data_.size() == num_elements);
    }

    int &operator()(size_t column, size_t row) {
        return data_[column + row * Size];
    }

    const int &operator()(size_t column, size_t row) const {
        return data_[column + row * Size];
    }

    using const_iterator = typename ivec::const_iterator;

    const_iterator begin() const {
        return data_.begin();
    }

    const_iterator end() const {
        return data_.end();
    }

    ivec &data() {
        return data_;
    }

    const ivec &data() const {
        return data_;
    }

    void data(ivec new_data) {
        data_ = move(new_data);
    }

private:
    ivec data_;
};

template<class T>
std::vector<T> make_iota_vector(size_t num_elements) {
    std::vector<T> result;
    result.resize(num_elements);
    iota(result.begin(), result.end(), T {0});
    return result;
}

template<size_t Size>
square_matrix<Size> make_iota_matrix() {
    square_matrix<Size> result;
    iota(result.data().begin(), result.data().end(), 0);
    return result;
}

template<size_t Size>
bool operator==(const square_matrix<Size> &lhs, const square_matrix<Size> &rhs) {
    return lhs.data() == rhs.data();
}

template<size_t Size>
bool operator!=(const square_matrix<Size> &lhs, const square_matrix<Size> &rhs) {
    return !(lhs == rhs);
}

using matrix_type = square_matrix<matrix_size>;

template<class T>
void check_vector_results(const std::string &description, const std::vector<T> &expected, const std::vector<T> &result) {
    auto cond = (expected == result);
    BOOST_CHECK(cond);
    if (!cond) {
        BOOST_ERROR(description << " failed.");
        std::cout << "Expected: " << std::endl;
        for (size_t i = 0; i < expected.size(); ++i) {
            std::cout << " " << expected[i];
        }
        std::cout << std::endl << "Received: " << std::endl;
        for (size_t i = 0; i < result.size(); ++i) {
            std::cout << " " << result[i];
        }
        std::cout << std::endl;
        std::cout << "Size: " << expected.size() << " vs. " << result.size() << std::endl;
        std::cout << "Differ at: " << std::endl;
        bool same = true;
        for (size_t i = 0; i < std::min(expected.size(), result.size()); ++i) {
            if (expected[i] != result[i]) {
                std::cout << "[" << i << "] " << expected[i] << " != " << result[i] << std::endl;
                same = false;
            }
        }
        if (same) {
            std::cout << "... nowhere." << std::endl;
        }
    }
}

template<class T>
void check_mref_results(const std::string &description, const std::vector<T> &expected, mem_ref<T> &result) {
    auto exp_res = result.data();
    BOOST_REQUIRE(exp_res);
    auto res = *exp_res;
    auto cond = (expected == res);
    BOOST_CHECK(cond);
    if (!cond) {
        BOOST_ERROR(description << " failed.");
        std::cout << "Expected: " << std::endl;
        for (size_t i = 0; i < expected.size(); ++i) {
            std::cout << " " << expected[i];
        }
        std::cout << std::endl << "Received: " << std::endl;
        for (size_t i = 0; i < res.size(); ++i) {
            std::cout << " " << res[i];
        }
        std::cout << std::endl;
    }
}

void test_opencl(spawner &sys) {
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device_if([](const device_ptr) { return true; });
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    const ivec expected1 {56, 62, 68, 74, 152, 174, 196, 218, 248, 286, 324, 362, 344, 398, 452, 506};
    auto w1 = mngr.spawn(prog, kn_matrix, opencl::nd_range {dims {matrix_size, matrix_size}}, opencl::in<int> {},
                         opencl::out<int> {});
    self->send(w1, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive(
        [&](const ivec &result) {
            check_vector_results(
                "Simple matrix multiplication using vectors"
                " (kernel wrapped in program)",
                expected1, result);
        },
        others >> wrong_msg);
    opencl::nd_range range2 {dims {matrix_size, matrix_size}};
    // Pass kernel directly to the actor
    auto w2 = mngr.spawn(kernel_source, kn_matrix, range2, opencl::in<int> {}, opencl::out<int> {});
    self->send(w2, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive(
        [&](const ivec &result) {
            check_vector_results(
                "Simple matrix multiplication using vectors"
                " (kernel passed directly)",
                expected1, result);
        },
        others >> wrong_msg);
    const matrix_type expected2(move(expected1));
    auto map_arg = [](message &msg) -> optional<message> {
        return msg.apply([](matrix_type &mx) { return make_message(move(mx.data())); });
    };
    auto map_res = [](ivec result) -> message { return make_message(matrix_type {move(result)}); };
    opencl::nd_range range3 {dims {matrix_size, matrix_size}};
    // let the runtime choose the device
    auto w3 = mngr.spawn(mngr.create_program(kernel_source), kn_matrix, range3, map_arg, map_res, opencl::in<int> {},
                         opencl::out<int> {});
    self->send(w3, make_iota_matrix<matrix_size>());
    self->receive(
        [&](const matrix_type &result) {
            check_vector_results(
                "Matrix multiplication with user defined type "
                "(kernel wrapped in program)",
                expected2.data(), result.data());
        },
        others >> wrong_msg);
    opencl::nd_range range4 {dims {matrix_size, matrix_size}};
    auto w4 = mngr.spawn(prog, kn_matrix, range4, map_arg, map_res, opencl::in<int> {}, opencl::out<int> {});
    self->send(w4, make_iota_matrix<matrix_size>());
    self->receive(
        [&](const matrix_type &result) {
            check_vector_results("Matrix multiplication with user defined type", expected2.data(), result.data());
        },
        others >> wrong_msg);
#ifndef MTL_NO_EXCEPTIONS
    BOOST_TEST_MESSAGE(
        "Expecting exception (compiling invalid kernel, "
        "semicolon is missing).");
    try {
        /* auto expected_error = */ mngr.create_program(kernel_source_error);
    } catch (const std::exception &exc) {
        BOOST_TEST_MESSAGE("got: " << exc.what());
    }
#endif    // MTL_NO_EXCEPTIONS
    // create program with opencl compiler flags
    auto prog5 = mngr.create_program(kernel_source_compiler_flag, compiler_flag);
    opencl::nd_range range5 {dims {array_size}};
    auto w5 = mngr.spawn(prog5, kn_compiler_flag, range5, opencl::in<int> {}, opencl::out<int> {});
    self->send(w5, make_iota_vector<int>(array_size));
    auto expected3 = make_iota_vector<int>(array_size);
    self->receive([&](const ivec &result) { check_vector_results("Passing compiler flags", expected3, result); },
                  others >> wrong_msg);

    // test for manuel return size selection (max workgroup size 1d)
    auto max_wg_size = std::min(dev->max_work_item_sizes()[0], size_t {512});
    auto reduce_buffer_size = static_cast<size_t>(max_wg_size) * 8;
    auto reduce_local_size = static_cast<size_t>(max_wg_size);
    auto reduce_work_groups = reduce_buffer_size / reduce_local_size;
    auto reduce_global_size = reduce_buffer_size;
    auto reduce_result_size = reduce_work_groups;
    ivec arr6(reduce_buffer_size);
    int n = static_cast<int>(arr6.capacity());
    generate(arr6.begin(), arr6.end(), [&] { return --n; });
    opencl::nd_range range6 {dims {reduce_global_size}, dims {},    // no offset
                             dims {reduce_local_size}};
    auto result_size_6 = [reduce_result_size](const ivec &) { return reduce_result_size; };
    auto w6 = mngr.spawn(prog, kn_reduce, range6, opencl::in<int> {}, opencl::out<int> {result_size_6});
    self->send(w6, move(arr6));
    auto wg_size_as_int = static_cast<int>(max_wg_size);
    ivec expected4 {wg_size_as_int * 7, wg_size_as_int * 6, wg_size_as_int * 5, wg_size_as_int * 4,
                    wg_size_as_int * 3, wg_size_as_int * 2, wg_size_as_int,     0};
    self->receive([&](const ivec &result) { check_vector_results("Passing size for the output", expected4, result); },
                  others >> wrong_msg);

    // calculator function for getting the size of the output
    auto result_size_7 = [](const ivec &) { return problem_size; };
    // constant memory arguments
    const ivec arr7 {static_cast<int>(problem_size)};
    auto w7 = mngr.spawn(kernel_source, kn_const, opencl::nd_range {dims {problem_size}}, opencl::in<int> {},
                         opencl::out<int> {result_size_7});
    self->send(w7, move(arr7));
    ivec expected5(problem_size);
    fill(begin(expected5), end(expected5), static_cast<int>(problem_size));
    self->receive([&](const ivec &result) { check_vector_results("Using const input argument", expected5, result); },
                  others >> wrong_msg);
}

void test_arguments(spawner &sys) {
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device_if([](const device_ptr) { return true; });
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    const ivec expected1 {56, 62, 68, 74, 152, 174, 196, 218, 248, 286, 324, 362, 344, 398, 452, 506};
    auto w1 = mngr.spawn(mngr.create_program(kernel_source, "", dev), kn_matrix,
                         opencl::nd_range {dims {matrix_size, matrix_size}}, opencl::in<int> {}, opencl::out<int> {});
    self->send(w1, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive([&](const ivec &result) { check_vector_results("arguments: from in to out", expected1, result); },
                  others >> wrong_msg);
    ivec input9 = make_iota_vector<int>(problem_size);
    ivec expected9 {input9};
    for_each(begin(expected9), end(expected9), [](int &val) { val *= 2; });
    auto w9 = mngr.spawn(kernel_source, kn_inout, nd_range {dims {problem_size}}, opencl::in_out<int> {});
    self->send(w9, move(input9));
    self->receive([&](const ivec &result) { check_vector_results("Testing in_out arugment", expected9, result); },
                  others >> wrong_msg);
    ivec input10 = make_iota_vector<int>(problem_size);
    ivec expected10 {input10};
    for_each(begin(expected10), end(expected10), [](int &val) { val *= 2; });
    auto result_size_10 = [=](const ivec &input) { return input.size(); };
    auto w10 = mngr.spawn(kernel_source, kn_scratch, nd_range {dims {problem_size}}, opencl::in_out<int> {},
                          opencl::scratch<int> {result_size_10});
    self->send(w10, move(input10));
    self->receive([&](const ivec &result) { check_vector_results("Testing buffer arugment", expected10, result); },
                  others >> wrong_msg);
    // test local
    size_t la_global = 256;
    size_t la_local = la_global / 2;
    ivec input_local = make_iota_vector<int>(la_global);
    ivec expected_local {input_local};
    auto last = 0;
    for (size_t i = 0; i < la_global; ++i) {
        if (i == la_local) {
            last = 0;
        }
        auto tmp = expected_local[i];
        expected_local[i] = last;
        last += tmp;
    }
    auto work_local = mngr.spawn(kernel_source, kn_local, nd_range {dims {la_global}, {}, dims {la_local}},
                                 opencl::in_out<int> {}, opencl::local<int> {la_local});
    self->send(work_local, std::move(input_local));
    self->receive([&](const ivec &result) { check_vector_results("Testing local arugment", expected_local, result); });
    // Same test, different argument order
    input_local = make_iota_vector<int>(la_global);
    work_local = mngr.spawn(kernel_source, kn_order, nd_range {dims {la_global}, {}, dims {la_local}},
                            opencl::local<int> {la_local}, opencl::in_out<int> {});
    self->send(work_local, std::move(input_local));
    self->receive([&](const ivec &result) { check_vector_results("Testing local arugment", expected_local, result); });
    // Test private argument
    ivec input_private = make_iota_vector<int>(problem_size);
    int val_private = 42;
    ivec expected_private {input_private};
    for_each(begin(expected_private), end(expected_private), [val_private](int &val) { val += val_private; });
    auto worker_private = mngr.spawn(kernel_source, kn_private, nd_range {dims {problem_size}}, opencl::in_out<int> {},
                                     opencl::priv<int> {val_private});
    self->send(worker_private, std::move(input_private));
    self->receive(
        [&](const ivec &result) { check_vector_results("Testing private arugment", expected_private, result); });
}

BOOST_AUTO_TEST_CASE(opencl_basics_test) {
    spawner_config cfg;
    cfg.load<opencl::manager>().add_message_type<ivec>("int_vector").add_message_type<matrix_type>("square_matrix");
    spawner system {cfg};
    test_opencl(system);
    system.await_all_actors_done();
}

BOOST_AUTO_TEST_CASE(opencl_arguments_test) {
    spawner_config cfg;
    cfg.load<opencl::manager>().add_message_type<ivec>("int_vector").add_message_type<matrix_type>("square_matrix");
    spawner system {cfg};
    test_arguments(system);
    system.await_all_actors_done();
}

BOOST_AUTO_TEST_CASE(opencl_mem_refs_test) {
    spawner_config cfg;
    cfg.load<opencl::manager>();
    spawner system {cfg};
    auto &mngr = system.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    // global arguments
    std::vector<uint32_t> input {1, 2, 3, 4};
    auto buf_1 = dev->global_argument(input, buffer_type::input_output);
    BOOST_CHECK_EQUAL(buf_1.size(), input.size());
    auto res_1 = buf_1.data();
    BOOST_CHECK(res_1);
    BOOST_CHECK_EQUAL(res_1->size(), input.size());
    check_vector_results("Testing mem_ref", input, *res_1);
    auto res_2 = buf_1.data(2ul);
    BOOST_CHECK(res_2);
    BOOST_CHECK_EQUAL(res_2->size(), 2ul);
    BOOST_CHECK_EQUAL((*res_2)[0], input[0]);
    BOOST_CHECK_EQUAL((*res_2)[1], input[1]);
    std::vector<uint32_t> new_input {1, 2, 3, 4, 5};
    buf_1 = dev->global_argument(new_input, buffer_type::input_output);
    BOOST_CHECK_EQUAL(buf_1.size(), new_input.size());
    auto res_3 = buf_1.data();
    BOOST_CHECK(res_3);
    mem_ref<uint32_t> buf_2 {std::move(buf_1)};
    BOOST_CHECK_EQUAL(buf_2.size(), new_input.size());
    auto res_4 = buf_2.data();
    BOOST_CHECK(res_4);
    buf_2.reset();
    auto res_5 = buf_2.data();
    BOOST_CHECK(!res_5);
}

BOOST_AUTO_TEST_CASE(opencl_argument_info_test) {
    using base_t = int;
    using in_arg_t = ::type_list<opencl::in<base_t>>;
    using in_arg_info_t = typename cl_arg_info_list<in_arg_t>::type;
    using in_arg_wrap_t = typename ::tl_head<in_arg_info_t>::type;
    static_assert(in_arg_wrap_t::in_pos == 0, "In-index for `in` wrong.");
    static_assert(in_arg_wrap_t::out_pos == -1, "Out-index for `in` wrong.");
    using out_arg_t = ::type_list<opencl::out<base_t>>;
    using out_arg_info_t = typename cl_arg_info_list<out_arg_t>::type;
    using out_arg_wrap_t = typename ::tl_head<out_arg_info_t>::type;
    static_assert(out_arg_wrap_t::in_pos == -1, "In-index for `out` wrong.");
    static_assert(out_arg_wrap_t::out_pos == 0, "Out-index for `out` wrong.");
    using io_arg_t = ::type_list<opencl::in_out<base_t>>;
    using io_arg_info_t = typename cl_arg_info_list<io_arg_t>::type;
    using io_arg_wrap_t = typename ::tl_head<io_arg_info_t>::type;
    static_assert(io_arg_wrap_t::in_pos == 0, "In-index for `in_out` wrong.");
    static_assert(io_arg_wrap_t::out_pos == 0, "Out-index for `in_out` wrong.");
    using arg_list_t = ::type_list<opencl::in<base_t>, opencl::out<base_t>, opencl::local<base_t>,
                                   opencl::in_out<base_t>, opencl::priv<base_t>, opencl::priv<base_t, val>>;
    using arg_info_list_t = typename cl_arg_info_list<arg_list_t>::type;
    using arg_info_0_t = typename ::tl_at<arg_info_list_t, 0>::type;
    static_assert(arg_info_0_t::in_pos == 0, "In-index for `in` wrong.");
    static_assert(arg_info_0_t::out_pos == -1, "Out-index for `in` wrong.");
    using arg_info_1_t = typename ::tl_at<arg_info_list_t, 1>::type;
    static_assert(arg_info_1_t::in_pos == -1, "In-index for `out` wrong.");
    static_assert(arg_info_1_t::out_pos == 0, "Out-index for `out` wrong.");
    using arg_info_2_t = typename ::tl_at<arg_info_list_t, 2>::type;
    static_assert(arg_info_2_t::in_pos == -1, "In-index for `local` wrong.");
    static_assert(arg_info_2_t::out_pos == -1, "Out-index for `local` wrong.");
    using arg_info_3_t = typename ::tl_at<arg_info_list_t, 3>::type;
    static_assert(arg_info_3_t::in_pos == 1, "In-index for `in_out` wrong.");
    static_assert(arg_info_3_t::out_pos == 1, "Out-index for `in_out` wrong.");
    using arg_info_4_t = typename ::tl_at<arg_info_list_t, 4>::type;
    static_assert(arg_info_4_t::in_pos == -1, "In-index for `priv` wrong.");
    static_assert(arg_info_4_t::out_pos == -1, "Out-index for `priv` wrong.");
    using arg_info_5_t = typename ::tl_at<arg_info_list_t, 5>::type;
    static_assert(arg_info_5_t::in_pos == 2, "In-index for `priv` wrong.");
    static_assert(arg_info_5_t::out_pos == -1, "Out-index for `priv` wrong.");
    // gives the test some output.
    BOOST_CHECK_EQUAL(true, true);
}

void test_in_val_out_val(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing in: val  -> out: val ");
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    const ivec res1 {56, 62, 68, 74, 152, 174, 196, 218, 248, 286, 324, 362, 344, 398, 452, 506};
    auto conf = opencl::nd_range {dims {matrix_size, matrix_size}};
    auto w1 = mngr.spawn(prog, kn_matrix, conf, in<int> {}, out<int> {});
    self->send(w1, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive(
        [&](const ivec &result) {
            check_vector_results(
                "Simple matrix multiplication using vectors"
                " (kernel wrapped in program)",
                res1, result);
        },
        others >> wrong_msg);
    // Pass kernel directly to the actor
    auto w2 = mngr.spawn(kernel_source, kn_matrix, conf, in<int> {}, out<int> {});
    self->send(w2, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive(
        [&](const ivec &result) {
            check_vector_results(
                "Simple matrix multiplication using vectors"
                " (kernel passed directly)",
                res1, result);
        },
        others >> wrong_msg);
    // Wrap message in user-defined type and use mapping functions
    const matrix_type res2(move(res1));
    auto map_arg = [](message &msg) -> optional<message> {
        return msg.apply([](matrix_type &mx) { return make_message(move(mx.data())); });
    };
    auto map_res = [](ivec result) -> message { return make_message(matrix_type {move(result)}); };
    auto w3 = mngr.spawn(prog, kn_matrix, conf, map_arg, map_res, in<int, val> {}, out<int, val> {});
    self->send(w3, make_iota_matrix<matrix_size>());
    self->receive(
        [&](const matrix_type &result) {
            check_vector_results(
                "Matrix multiplication with user defined type "
                "(kernel wrapped in program)",
                res2.data(), result.data());
        },
        others >> wrong_msg);
    // create program with opencl compiler flags
    auto prog2 = mngr.create_program(kernel_source_compiler_flag, compiler_flag);
    nd_range range2 {dims {array_size}};
    auto w4 = mngr.spawn(prog2, kn_compiler_flag, range2, in<int> {}, out<int> {});
    self->send(w4, make_iota_vector<int>(array_size));
    auto res3 = make_iota_vector<int>(array_size);
    self->receive([&](const ivec &result) { check_vector_results("Passing compiler flags", res3, result); },
                  others >> wrong_msg);

    // test for manuel return size selection (max workgroup size 1d)
    auto max_wg_size = std::min(dev->max_work_item_sizes()[0], size_t {512});
    auto reduce_buffer_size = static_cast<size_t>(max_wg_size) * 8;
    auto reduce_local_size = static_cast<size_t>(max_wg_size);
    auto reduce_work_groups = reduce_buffer_size / reduce_local_size;
    auto reduce_global_size = reduce_buffer_size;
    auto reduce_result_size = reduce_work_groups;
    ivec input(reduce_buffer_size);
    int n = static_cast<int>(input.capacity());
    generate(input.begin(), input.end(), [&] { return --n; });
    nd_range range3 {dims {reduce_global_size}, dims {}, dims {reduce_local_size}};
    auto res_size = [&](const ivec &) { return reduce_result_size; };
    auto w5 = mngr.spawn(prog, kn_reduce, range3, in<int> {}, out<int> {res_size});
    self->send(w5, move(input));
    auto wg_size_as_int = static_cast<int>(max_wg_size);
    ivec res4 {wg_size_as_int * 7, wg_size_as_int * 6, wg_size_as_int * 5, wg_size_as_int * 4,
               wg_size_as_int * 3, wg_size_as_int * 2, wg_size_as_int,     0};
    self->receive([&](const ivec &result) { check_vector_results("Passing size for the output", res4, result); },
                  others >> wrong_msg);
    // calculator function for getting the size of the output
    auto res_size2 = [](const ivec &) { return problem_size; };
    // constant memory arguments
    const ivec input2 {static_cast<int>(problem_size)};
    auto w6 = mngr.spawn(kernel_source, kn_const, nd_range {dims {problem_size}}, in<int> {}, out<int> {res_size2});
    self->send(w6, move(input2));
    ivec res5(problem_size);
    fill(begin(res5), end(res5), static_cast<int>(problem_size));
    self->receive([&](const ivec &result) { check_vector_results("Using const input argument", res5, result); },
                  others >> wrong_msg);
}

void test_in_val_out_mref(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing in: val  -> out: mref");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    const ivec res1 {56, 62, 68, 74, 152, 174, 196, 218, 248, 286, 324, 362, 344, 398, 452, 506};
    auto range = opencl::nd_range {dims {matrix_size, matrix_size}};
    auto w1 = mngr.spawn(prog, kn_matrix, range, in<int> {}, out<int, mref> {});
    self->send(w1, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive(
        [&](iref &result) {
            check_mref_results(
                "Simple matrix multiplication using vectors"
                " (kernel wrapped in program)",
                res1, result);
        },
        others >> wrong_msg);
    // Pass kernel directly to the actor
    auto w2 = mngr.spawn(kernel_source, kn_matrix, range, in<int> {}, out<int, mref> {});
    self->send(w2, make_iota_vector<int>(matrix_size * matrix_size));
    self->receive(
        [&](iref &result) {
            check_mref_results(
                "Simple matrix multiplication using vectors"
                " (kernel passed directly)",
                res1, result);
        },
        others >> wrong_msg);
    // test for manuel return size selection (max workgroup size 1d)
    auto max_wg_size = std::min(dev->max_work_item_sizes()[0], size_t {512});
    auto reduce_buffer_size = static_cast<size_t>(max_wg_size) * 8;
    auto reduce_local_size = static_cast<size_t>(max_wg_size);
    auto reduce_work_groups = reduce_buffer_size / reduce_local_size;
    auto reduce_global_size = reduce_buffer_size;
    auto reduce_result_size = reduce_work_groups;
    ivec input(reduce_buffer_size);
    int n = static_cast<int>(input.capacity());
    generate(input.begin(), input.end(), [&] { return --n; });
    nd_range range3 {dims {reduce_global_size}, dims {}, dims {reduce_local_size}};
    auto res_size = [&](const ivec &) { return reduce_result_size; };
    auto w5 = mngr.spawn(prog, kn_reduce, range3, in<int> {}, out<int, mref> {res_size});
    self->send(w5, move(input));
    auto wg_size_as_int = static_cast<int>(max_wg_size);
    ivec res4 {wg_size_as_int * 7, wg_size_as_int * 6, wg_size_as_int * 5, wg_size_as_int * 4,
               wg_size_as_int * 3, wg_size_as_int * 2, wg_size_as_int * 1, wg_size_as_int * 0};
    self->receive([&](iref &result) { check_mref_results("Passing size for the output", res4, result); },
                  others >> wrong_msg);
}

void test_in_mref_out_val(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing in: mref -> out: val ");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    const ivec res1 {56, 62, 68, 74, 152, 174, 196, 218, 248, 286, 324, 362, 344, 398, 452, 506};
    auto range = opencl::nd_range {dims {matrix_size, matrix_size}};
    auto w1 = mngr.spawn(prog, kn_matrix, range, in<int, mref> {}, out<int> {});
    auto matrix1 = make_iota_vector<int>(matrix_size * matrix_size);
    auto input1 = dev->global_argument(matrix1);
    self->send(w1, input1);
    self->receive(
        [&](const ivec &result) {
            check_vector_results(
                "Simple matrix multiplication using vectors"
                " (kernel wrapped in program)",
                res1, result);
        },
        others >> wrong_msg);
    // Pass kernel directly to the actor
    auto w2 = mngr.spawn(kernel_source, kn_matrix, range, in<int, mref> {}, out<int, val> {});
    self->send(w2, input1);
    self->receive(
        [&](const ivec &result) {
            check_vector_results(
                "Simple matrix multiplication using vectors"
                " (kernel passed directly)",
                res1, result);
        },
        others >> wrong_msg);
    // test for manuel return size selection (max workgroup size 1d)
    auto max_wg_size = std::min(dev->max_work_item_sizes()[0], size_t {512});
    auto reduce_buffer_size = static_cast<size_t>(max_wg_size) * 8;
    auto reduce_local_size = static_cast<size_t>(max_wg_size);
    auto reduce_work_groups = reduce_buffer_size / reduce_local_size;
    auto reduce_global_size = reduce_buffer_size;
    auto reduce_result_size = reduce_work_groups;
    ivec values(reduce_buffer_size);
    int n = static_cast<int>(values.capacity());
    generate(values.begin(), values.end(), [&] { return --n; });
    nd_range range3 {dims {reduce_global_size}, dims {}, dims {reduce_local_size}};
    auto res_size = [&](const iref &) { return reduce_result_size; };
    auto w5 = mngr.spawn(prog, kn_reduce, range3, in<int, mref> {}, out<int> {res_size});
    auto input2 = dev->global_argument(values);
    self->send(w5, input2);
    auto multiplier = static_cast<int>(max_wg_size);
    ivec res4 {multiplier * 7, multiplier * 6, multiplier * 5, multiplier * 4,
               multiplier * 3, multiplier * 2, multiplier * 1, multiplier * 0};
    self->receive([&](const ivec &result) { check_vector_results("Passing size for the output", res4, result); },
                  others >> wrong_msg);
}

void test_in_mref_out_mref(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing in: mref -> out: mref");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    const ivec res1 {56, 62, 68, 74, 152, 174, 196, 218, 248, 286, 324, 362, 344, 398, 452, 506};
    auto range = opencl::nd_range {dims {matrix_size, matrix_size}};
    auto w1 = mngr.spawn(prog, kn_matrix, range, in<int, mref> {}, out<int, mref> {});
    auto matrix1 = make_iota_vector<int>(matrix_size * matrix_size);
    auto input1 = dev->global_argument(matrix1);
    self->send(w1, input1);
    self->receive(
        [&](iref &result) {
            check_mref_results(
                "Simple matrix multiplication using vectors"
                " (kernel wrapped in program)",
                res1, result);
        },
        others >> wrong_msg);
    // Pass kernel directly to the actor
    auto w2 = mngr.spawn(kernel_source, kn_matrix, range, in<int, mref> {}, out<int, mref> {});
    self->send(w2, input1);
    self->receive(
        [&](iref &result) {
            check_mref_results(
                "Simple matrix multiplication using vectors"
                " (kernel passed directly)",
                res1, result);
        },
        others >> wrong_msg);
    // test for manuel return size selection (max workgroup size 1d)
    auto max_wg_size = std::min(dev->max_work_item_sizes()[0], size_t {512});
    auto reduce_buffer_size = static_cast<size_t>(max_wg_size) * 8;
    auto reduce_local_size = static_cast<size_t>(max_wg_size);
    auto reduce_work_groups = reduce_buffer_size / reduce_local_size;
    auto reduce_global_size = reduce_buffer_size;
    auto reduce_result_size = reduce_work_groups;
    ivec values(reduce_buffer_size);
    int n = static_cast<int>(values.capacity());
    generate(values.begin(), values.end(), [&] { return --n; });
    nd_range range3 {dims {reduce_global_size}, dims {}, dims {reduce_local_size}};
    auto res_size = [&](const iref &) { return reduce_result_size; };
    auto w5 = mngr.spawn(prog, kn_reduce, range3, in<int, mref> {}, out<int, mref> {res_size});
    auto input2 = dev->global_argument(values);
    self->send(w5, input2);
    auto multiplier = static_cast<int>(max_wg_size);
    ivec res4 {multiplier * 7, multiplier * 6, multiplier * 5, multiplier * 4,
               multiplier * 3, multiplier * 2, multiplier,     0};
    self->receive([&](iref &result) { check_mref_results("Passing size for the output", res4, result); },
                  others >> wrong_msg);
}

void test_varying_arguments(spawner &sys) {
    BOOST_TEST_MESSAGE(
        "Testing varying argument order "
        "(Might fail on some integrated GPUs)");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    size_t size = 23;
    nd_range range {dims {size}};
    auto input1 = make_iota_vector<int>(size);
    auto input2 = dev->global_argument(input1);
    auto w1 = mngr.spawn(prog, kn_varying, range, in<int> {}, out<int> {}, in<int> {}, out<int> {});
    self->send(w1, input1, input1);
    self->receive(
        [&](const ivec &res1, const ivec &res2) {
            check_vector_results("Varying args (vec only), output 1", input1, res1);
            check_vector_results("Varying args (vec only), output 2", input1, res2);
        },
        others >> wrong_msg);
    auto w2 = mngr.spawn(prog, kn_varying, range, in<int, mref> {}, out<int> {}, in<int> {}, out<int, mref> {});
    self->send(w2, input2, input1);
    self->receive(
        [&](const ivec &res1, iref &res2) {
            check_vector_results("Varying args (vec), output 1", input1, res1);
            check_mref_results("Varying args (ref), output 2", input1, res2);
        },
        others >> wrong_msg);
}

void test_inout(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing in_out arguments");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    ivec input = make_iota_vector<int>(problem_size);
    auto input2 = dev->global_argument(input);
    auto input3 = dev->global_argument(input);
    ivec res {input};
    for_each(begin(res), end(res), [](int &val) { val *= 2; });
    auto range = nd_range {dims {problem_size}};
    auto w1 = mngr.spawn(kernel_source, kn_inout, range, in_out<int, val, val> {});
    self->send(w1, input);
    self->receive([&](const ivec &result) { check_vector_results("Testing in_out (val -> val)", res, result); },
                  others >> wrong_msg);
    auto w2 = mngr.spawn(kernel_source, kn_inout, range, in_out<int, val, mref> {});
    self->send(w2, input);
    self->receive([&](iref &result) { check_mref_results("Testing in_out (val -> mref)", res, result); },
                  others >> wrong_msg);
    auto w3 = mngr.spawn(kernel_source, kn_inout, range, in_out<int, mref, val> {});
    self->send(w3, input2);
    self->receive([&](const ivec &result) { check_vector_results("Testing in_out (mref -> val)", res, result); },
                  others >> wrong_msg);
    auto w4 = mngr.spawn(kernel_source, kn_inout, range, in_out<int, mref, mref> {});
    self->send(w4, input3);
    self->receive([&](iref &result) { check_mref_results("Testing in_out (mref -> mref)", res, result); },
                  others >> wrong_msg);
}

void test_priv(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing priv argument");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    nd_range range {dims {problem_size}};
    ivec input = make_iota_vector<int>(problem_size);
    int value = 42;
    ivec res {input};
    for_each(begin(res), end(res), [&](int &val) { val += value; });
    auto w1 = mngr.spawn(kernel_source, kn_private, range, in_out<int> {}, priv<int> {value});
    self->send(w1, input);
    self->receive([&](const ivec &result) { check_vector_results("Testing hidden private arugment", res, result); },
                  others >> wrong_msg);
    auto w2 = mngr.spawn(kernel_source, kn_private, range, in_out<int> {}, priv<int, val> {});
    self->send(w2, input, value);
    self->receive([&](const ivec &result) { check_vector_results("Testing val private arugment", res, result); },
                  others >> wrong_msg);
}

void test_local(spawner &sys) {
    BOOST_TEST_MESSAGE("Testing local argument");
    // setup
    auto &mngr = sys.opencl_manager();
    auto opt = mngr.find_device(0);
    BOOST_REQUIRE(opt);
    auto dev = *opt;
    auto prog = mngr.create_program(kernel_source, "", dev);
    scoped_actor self {sys};
    auto wrong_msg = [&](message_view &x) -> result<message> {
        BOOST_ERROR("unexpected message" << x.content().stringify());
        return sec::unexpected_message;
    };
    // tests
    size_t global_size = 256;
    size_t local_size = global_size / 2;
    ivec res = make_iota_vector<int>(global_size);
    auto last = 0;
    for (size_t i = 0; i < global_size; ++i) {
        if (i == local_size) {
            last = 0;
        }
        auto tmp = res[i];
        res[i] = last;
        last += tmp;
    }
    auto range = nd_range {dims {global_size}, {}, dims {local_size}};
    auto w = mngr.spawn(kernel_source, kn_local, range, in_out<int> {}, local<int> {local_size});
    self->send(w, make_iota_vector<int>(global_size));
    self->receive([&](const ivec &result) { check_vector_results("Testing local arugment", res, result); },
                  others >> wrong_msg);
    // Same test, different argument order
    w = mngr.spawn(kernel_source, kn_order, range, local<int> {local_size}, in_out<int> {});
    self->send(w, make_iota_vector<int>(global_size));
    self->receive([&](const ivec &result) { check_vector_results("Testing local arugment", res, result); },
                  others >> wrong_msg);
}

BOOST_AUTO_TEST_CASE(actor_facade_test) {
    spawner_config cfg;
    cfg.load<opencl::manager>().add_message_type<ivec>("int_vector").add_message_type<matrix_type>("square_matrix");
    spawner system {cfg};
    test_in_val_out_val(system);
    test_in_val_out_mref(system);
    test_in_mref_out_val(system);
    test_in_mref_out_mref(system);
    test_varying_arguments(system);
    test_inout(system);
    test_priv(system);
    test_local(system);
    system.await_all_actors_done();
}
