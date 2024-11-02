#include "gtest/gtest.h"

#include "integer/prime_generator.hpp"
//#include "boost/multiprecision/cpp_int.hpp"
//
//#include "integer/integer.hpp"

//using namespace boost::multiprecision;

TEST(PrimeGeneratorTest, SimpleTest) {
    // 768 / 4 bits
    auto result = PrimeGenerator<BigInt>::get_prime(192);
    std::cout << result.to_string() << std::endl;

//    auto res = PrimeGenerator<cpp_int>::get_prime(240);
//    std::cout << res << std::endl;
//    std::cout << msb(res) << std::endl;

    // FAIL();
}

TEST(PrimeGeneratorTest, PrimeTableTest) {
//    auto result = PrimeGenerator<cpp_int>::generate_primes(1000);
//    std::cout << result.size() << std::endl;
//     FAIL();
}