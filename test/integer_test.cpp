#include <random>
#include "gtest/gtest.h"

#include "integer/integer.hpp"

#include "boost/multiprecision/cpp_int.hpp"

using namespace boost::multiprecision;

// Function to generate a random large number
std::string generate_random_large_number(size_t digits) {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<int> dis(0, 9);

    std::string number;
    number.reserve(digits);

    // Ensure the first digit is not zero (unless the number is a single digit)
    number.push_back('1' + dis(gen) % 9); // First digit (1-9)

    for (size_t i = 1; i < digits; ++i) {
        number.push_back('0' + dis(gen)); // Subsequent digits (0-9)
    }

    return number;
}
TEST(IntegerTest, SimpleTest) {
    EXPECT_EQ(BigInt("1352465432131321").to_string(), "1352465432131321");
    EXPECT_EQ(BigInt("1000000000001").to_string(), "1000000000001");

    cpp_int test_boost = 1352465432131321;
}

TEST(IntegerTest, AddSimpleTest) {
    cpp_int num1(generate_random_large_number(50));
    cpp_int num2(generate_random_large_number(50));

    cpp_int sum = num1 + num2;

    BigInt big1(num1.str());
    BigInt big2(num2.str());

    BigInt result = big1 + big2;

    EXPECT_EQ(result.to_string(), sum.str());
}

TEST(IntegerTest, AddFuzzingTest) {
    for (int i = 0; i < 10; ++i) {
        cpp_int num1(generate_random_large_number(50));
        cpp_int num2(generate_random_large_number(50));

        cpp_int sum = num1 + num2;

        BigInt big1(num1.str());
        BigInt big2(num2.str());

        BigInt result = big1 + big2;

        EXPECT_EQ(result.to_string(), sum.str());
    }
}

TEST(IntegerTest, MultiplicationFuzzingTest) {
    for (int i = 0; i < 10; ++i) {
        cpp_int num1(generate_random_large_number(50));
        cpp_int num2(generate_random_large_number(50));

        cpp_int sum = num1 * num2;

        BigInt big1(num1.str());
        BigInt big2(num2.str());

        BigInt result = big1 * big2;

        EXPECT_EQ(result.to_string(), sum.str());
    }
}