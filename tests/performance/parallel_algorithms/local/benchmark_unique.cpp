///////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2017 Taeguk Kwon
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
///////////////////////////////////////////////////////////////////////////////

#include <hpx/hpx_init.hpp>
#include <hpx/hpx.hpp>
#include <hpx/include/parallel_copy.hpp>
#include <hpx/include/parallel_generate.hpp>
#include <hpx/include/parallel_unique.hpp>
#include <hpx/util/high_resolution_clock.hpp>
#include <hpx/util/lightweight_test.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/random.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <iostream>
#include <string>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
struct random_fill
{
    random_fill(std::size_t random_range)
        : gen(std::rand()),
        dist(0, random_range - 1)
    {}

    int operator()()
    {
        return dist(gen);
    }

    boost::random::mt19937 gen;
    boost::random::uniform_int_distribution<> dist;
};

///////////////////////////////////////////////////////////////////////////////
struct heavy_type
{
    heavy_type() = default;
    heavy_type(int rand_no)
    {
        vec_.reserve(vec_size_);
        for (std::size_t i = 0u; i < vec_size_; ++i)
            vec_.push_back(rand_no);
    }

    bool operator==(heavy_type const & t) const
    {
        return std::equal(std::begin(vec_), std::end(vec_),
            std::begin(t.vec_), std::end(t.vec_));
    }

    std::vector<int> vec_;
    static const std::size_t vec_size_{ 50 };
};

///////////////////////////////////////////////////////////////////////////////
template <typename OrgIter, typename InIter>
double run_unique_benchmark_std(int test_count,
    OrgIter org_first, OrgIter org_last, InIter first, InIter last)
{
    std::uint64_t time = std::uint64_t(0);

    for (int i = 0; i < test_count; ++i)
    {

        // Restore [first, last) with original data.
        hpx::parallel::copy(hpx::parallel::execution::par,
            org_first, org_last, first);
        std::uint64_t elapsed = hpx::util::high_resolution_clock::now();
        std::unique(first, last);
        time += hpx::util::high_resolution_clock::now() - elapsed;
    }

    return (time * 1e-9) / test_count;
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename OrgIter, typename FwdIter>
double run_unique_benchmark_hpx(int test_count, ExPolicy policy,
    OrgIter org_first, OrgIter org_last, FwdIter first, FwdIter last)
{
    std::uint64_t time = std::uint64_t(0);

    for (int i = 0; i < test_count; ++i)
    {

        // Restore [first, last) with original data.
        hpx::parallel::copy(hpx::parallel::execution::par,
            org_first, org_last, first);
        std::uint64_t elapsed = hpx::util::high_resolution_clock::now();
        hpx::parallel::unique(policy, first, last);
        time += hpx::util::high_resolution_clock::now() - elapsed;
    }

    return (time * 1e-9) / test_count;
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag, typename DataType>
void run_benchmark(std::size_t vector_size, int test_count,
    std::size_t random_range, IteratorTag, DataType)
{
    std::cout << "* Preparing Benchmark..." << std::endl;

    typedef typename std::vector<DataType>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<DataType> v(vector_size), org_v;
    iterator first = iterator(std::begin(v));
    iterator last = iterator(std::end(v));

    // initialize data
    using namespace hpx::parallel;
    generate(execution::par, std::begin(v), std::end(v),
        random_fill(random_range));
    org_v = v;

    auto dest_dist = std::distance(first.base(),
        std::unique(first, last).base());

    auto org_first = std::begin(org_v);
    auto org_last = std::end(org_v);

    std::cout << "*** Destination iterator distance : "
        << dest_dist << std::endl << std::endl;

    std::cout << "* Running Benchmark..." << std::endl;
    std::cout << "--- run_unique_benchmark_std ---" << std::endl;
    double time_std =
        run_unique_benchmark_std(test_count,
            org_first, org_last, first, last);

    std::cout << "--- run_unique_benchmark_seq ---" << std::endl;
    double time_seq =
        run_unique_benchmark_hpx(test_count, execution::seq,
            org_first, org_last, first, last);

    std::cout << "--- run_unique_benchmark_par ---" << std::endl;
    double time_par =
        run_unique_benchmark_hpx(test_count, execution::par,
            org_first, org_last, first, last);

    std::cout << "--- run_unique_benchmark_par_unseq ---" << std::endl;
    double time_par_unseq =
        run_unique_benchmark_hpx(test_count, execution::par_unseq,
            org_first, org_last, first, last);

    std::cout << "\n-------------- Benchmark Result --------------" << std::endl;
    auto fmt = "unique (%1%) : %2%(sec)";
    std::cout << (boost::format(fmt) % "std" % time_std) << std::endl;
    std::cout << (boost::format(fmt) % "seq" % time_seq) << std::endl;
    std::cout << (boost::format(fmt) % "par" % time_par) << std::endl;
    std::cout << (boost::format(fmt) % "par_unseq" % time_par_unseq) << std::endl;
    std::cout << "----------------------------------------------" << std::endl;
}

template <typename IteratorTag>
void run_benchmark(std::size_t vector_size, int test_count,
    std::size_t random_range,
    IteratorTag iterator_tag, std::string const& data_type_str)
{
    if (data_type_str == "light")
        run_benchmark(vector_size, test_count, random_range,
            iterator_tag, int());
    else // heavy
        run_benchmark(vector_size, test_count, random_range,
            iterator_tag, heavy_type());
}

void run_benchmark(std::size_t vector_size, int test_count,
    std::size_t random_range,
    std::string const& iterator_tag_str, std::string const& data_type_str)
{
    if (iterator_tag_str == "random")
        run_benchmark(vector_size, test_count, random_range,
            std::random_access_iterator_tag(), data_type_str);
    else if (iterator_tag_str == "bidirectional")
        run_benchmark(vector_size, test_count, random_range,
            std::bidirectional_iterator_tag(), data_type_str);
    else // forward
        run_benchmark(vector_size, test_count, random_range,
            std::forward_iterator_tag(), data_type_str);
}

///////////////////////////////////////////////////////////////////////////////
std::string correct_iterator_tag_str(std::string const& iterator_tag)
{
    if (iterator_tag != "random" &&
        iterator_tag != "bidirectional" &&
        iterator_tag != "forward")
        return "random";
    else
        return iterator_tag;
}

std::string correct_data_type_str(std::string const& data_type)
{
    if (data_type != "light" &&
        data_type != "heavy")
        return "light";
    else
        return data_type;
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(boost::program_options::variables_map& vm)
{
    unsigned int seed = (unsigned int)std::time(nullptr);
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::srand(seed);

    // pull values from cmd
    std::size_t vector_size = vm["vector_size"].as<std::size_t>();
    std::size_t random_range = vm["random_range"].as<std::size_t>();
    int test_count = vm["test_count"].as<int>();
    std::string iterator_tag_str = correct_iterator_tag_str(
        vm["iterator_tag"].as<std::string>());
    std::string data_type_str = correct_data_type_str(
        vm["data_type"].as<std::string>());

    std::size_t const os_threads = hpx::get_os_thread_count();

    if (random_range < 1)
        random_range = 1;

    std::cout << "-------------- Benchmark Config --------------" << std::endl;
    std::cout << "seed         : " << seed << std::endl;
    std::cout << "vector_size  : " << vector_size << std::endl;
    std::cout << "random_range : " << random_range << std::endl;
    std::cout << "iterator_tag : " << iterator_tag_str << std::endl;
    std::cout << "data_type    : " << data_type_str << std::endl;
    std::cout << "test_count   : " << test_count << std::endl;
    std::cout << "os threads   : " << os_threads << std::endl;
    std::cout << "----------------------------------------------\n" << std::endl;

    run_benchmark(vector_size, test_count, random_range,
        iterator_tag_str, data_type_str);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    using namespace boost::program_options;
    options_description desc_commandline(
        "usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()
        ("vector_size",
            boost::program_options::value<std::size_t>()->default_value(1000000),
            "size of vector (default: 1000000)")
        ("random_range",
            boost::program_options::value<std::size_t>()->default_value(6),
            "range of random numbers [0, x) (default: 6)")
        ("iterator_tag",
            boost::program_options::value<std::string>()->default_value("random"),
            "the kind of iterator tag (random/bidirectional/forward)")
        ("data_type",
            boost::program_options::value<std::string>()->default_value("light"),
            "the kind of data type (light/heavy)")
        ("test_count",
            boost::program_options::value<int>()->default_value(10),
            "number of tests to be averaged (default: 10)")
        ("seed,s", boost::program_options::value<unsigned int>(),
            "the random number generator seed to use for this run")
        ;

    // initialize program
    std::vector<std::string> const cfg = {
        "hpx.os_threads=all"
    };

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}
