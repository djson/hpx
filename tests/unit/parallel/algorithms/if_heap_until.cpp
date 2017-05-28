//  Copyright (c) 2017 Taeguk Kwon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/include/parallel_is_heap.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <boost/range/functions.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include "is_heap_until_tests.hpp"
#include "test_utils.hpp"

////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_is_heap_until()
{
    using namespace hpx::parallel;

    test_is_heap_until(execution::seq, IteratorTag());
    test_is_heap_until(execution::par, IteratorTag());
    test_is_heap_until(execution::par_unseq, IteratorTag());

    test_is_heap_until(execution::seq, IteratorTag(), user_defined_type());
    test_is_heap_until(execution::par, IteratorTag(), user_defined_type());
    test_is_heap_until(execution::par_unseq, IteratorTag(), user_defined_type());

    test_is_heap_until(execution::seq, IteratorTag(), int(), std::greater<int>());
    test_is_heap_until(execution::par, IteratorTag(), int(), std::less<int>());
    test_is_heap_until(execution::par_unseq, IteratorTag(), int(), std::greater_equal<int>());

    test_is_heap_until_async(execution::seq(execution::task), IteratorTag());
    test_is_heap_until_async(execution::par(execution::task), IteratorTag());

    test_is_heap_until_async(execution::seq(execution::task), IteratorTag(), user_defined_type());
    test_is_heap_until_async(execution::par(execution::task), IteratorTag(), user_defined_type());
}

void is_heap_until_test()
{
    std::cout << "--- is_heap_until_test ---" << std::endl;
    test_is_heap_until<std::random_access_iterator_tag>();
}

template <typename IteratorTag>
void test_is_heap_until_exception()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_is_heap_until_exception(execution::seq, IteratorTag());
    test_is_heap_until_exception(execution::par, IteratorTag());

    test_is_heap_until_exception_async(execution::seq(execution::task), IteratorTag());
    test_is_heap_until_exception_async(execution::par(execution::task), IteratorTag());
}

void is_heap_until_exception_test()
{
    std::cout << "--- is_heap_until_exception_test ---" << std::endl;
    test_is_heap_until_exception<std::random_access_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_is_heap_until_bad_alloc()
{
    using namespace hpx::parallel;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_is_heap_until_bad_alloc(execution::seq, IteratorTag());
    test_is_heap_until_bad_alloc(execution::par, IteratorTag());

    test_is_heap_until_bad_alloc_async(execution::seq(execution::task), IteratorTag());
    test_is_heap_until_bad_alloc_async(execution::par(execution::task), IteratorTag());
}

void is_heap_until_bad_alloc_test()
{
    std::cout << "--- is_heap_until_bad_alloc_test ---" << std::endl;
    test_is_heap_until_bad_alloc<std::random_access_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(boost::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int)std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    std::srand(seed);

    is_heap_until_test();
    is_heap_until_exception_test();
    is_heap_until_bad_alloc_test();

    std::cout << "Test Finish!" << std::endl;

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace boost::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()
        ("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run")
        ;

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {
        "hpx.os_threads=all"
    };

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}