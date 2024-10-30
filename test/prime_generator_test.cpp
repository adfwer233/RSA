#include "gtest/gtest.h"

#include "integer/prime_generator.hpp"
#include "boost/multiprecision/cpp_int.hpp"

using namespace boost::multiprecision;

TEST(PrimeGeneratorTest, SimpleTest) {
     auto result = PrimeGenerator<cpp_int>::get_prime(1024);
     std::cout << result << std::endl;
    // FAIL();
}

TEST(PrimeGeneratorTest, PrimeTableTest) {
    auto result = PrimeGenerator<cpp_int>::generate_primes(1000);
    std::cout << result.size() << std::endl;
//     FAIL();
}