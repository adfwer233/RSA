#include "gtest/gtest.h"

#include "integer/prime_generator.hpp"
#include "boost/multiprecision/cpp_int.hpp"

#include "integer/integer.hpp"

using namespace boost::multiprecision;

TEST(PrimeGeneratorTest, SimpleTest) {
    // apprximated 1000 bits
    auto result = PrimeGenerator<BigInt>::get_prime(300);
    std::cout << result.to_string() << std::endl;
    // FAIL();
}

TEST(PrimeGeneratorTest, PrimeTableTest) {
    auto result = PrimeGenerator<cpp_int>::generate_primes(1000);
    std::cout << result.size() << std::endl;
//     FAIL();
}