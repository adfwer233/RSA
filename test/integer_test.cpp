#include <random>
#include "gtest/gtest.h"

#include "integer/integer.hpp"

#include "boost/multiprecision/cpp_int.hpp"

using namespace boost::multiprecision;

// Function to generate a random large number in hex

enum class DigitFormat {
    dec, hex
};

template<DigitFormat Format = DigitFormat::hex>
std::string generate_random_large_number(size_t digits) {
    static const char hex_chars[] = "0123456789abcdef";
    std::string result;
    result.reserve(digits);

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 15); // Range for hex characters
    std::uniform_int_distribution<int> dist_non_zero(1, 15); // Range for hex characters

    // Generate each hex digit
    for (size_t i = 0; i < digits; ++i) {
        if (i == 0)
            result += hex_chars[dist_non_zero(gen)];
        else
            result += hex_chars[dist(gen)];
    }

    if constexpr (Format == DigitFormat::hex) {
        return "0x" + result;
    } else {
        std::stringstream ss;
        ss << std::hex << "0x" + result;
        ss >> std::dec >> result;

        return result;
    }
}

std::string convert_hex_to_dec(const std::string_view hex_value) {
    std::string result;
    std::stringstream ss;
    ss << std::hex << hex_value;

    cpp_int test;
    ss >> test;

    return test.str();
}

TEST(IntegerTest, SimpleTest) {
    EXPECT_EQ(BigInt("0x1352465432131321").to_string(), "0x1352465432131321");
    EXPECT_EQ(BigInt("0x1000000000001").to_string(), "0x1000000000001");

    for (int i = 0; i < 10; i++) {
        auto rd = generate_random_large_number(10);
        EXPECT_EQ(BigInt(rd).to_string(), rd);
    }
}

TEST(IntegerTest, AddSimpleTest) {
    std::string rd1 = generate_random_large_number(50);
    std::string rd2 = generate_random_large_number(50);
    cpp_int num1(convert_hex_to_dec(rd1));
    cpp_int num2(convert_hex_to_dec(rd2));

    cpp_int sum = num1 + num2;

    BigInt big1(rd1);
    BigInt big2(rd2);

    BigInt result = big1 + big2;

    EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
}

TEST(IntegerTest, AddFuzzingTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(50);
        std::string rd2 = generate_random_large_number(50);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        cpp_int sum = num1 + num2;

        BigInt big1(rd1);
        BigInt big2(rd2);

        BigInt result = big1 + big2;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }
}

TEST(IntegerTest, SubtractTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(50);
        std::string rd2 = generate_random_large_number(50);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        if (num1 < num2) {
            std::swap(rd1, rd2);
            std::swap(num1, num2);
        }

        cpp_int sum = num1 - num2;

        BigInt big1(rd1);
        BigInt big2(rd2);

        BigInt result = big1 - big2;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }
}


TEST(IntegerTest, MultiplicationTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(50);
        std::string rd2 = generate_random_large_number(50);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        cpp_int sum = num1 * num2;

        BigInt big1(rd1);
        BigInt big2(rd2);

        BigInt result = big1 * big2;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }
}

TEST(IntegerTest, MultiplicationBenchmark) {
    double boost_sum = 0;
    double our_sum = 0;

    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(2048);
        std::string rd2 = generate_random_large_number(1024);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        auto boost_start = std::chrono::steady_clock::now();
        cpp_int sum = num1 / num2;
        auto boost_end = std::chrono::steady_clock::now();

        double duration_boost = std::chrono::duration<double, std::nano>(boost_end - boost_start).count();
        boost_sum += duration_boost;

        BigInt big1(rd1);
        BigInt big2(rd2);

        auto our_start = std::chrono::steady_clock::now();
        BigInt result = big1 / big2;
        auto our_end = std::chrono::steady_clock::now();

        our_sum += std::chrono::duration<double, std::nano>(our_end - our_start).count();

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }

    std::cout << boost_sum << std::endl << our_sum << std::endl;
}

TEST(IntegerTest, DivisionTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(50);
        std::string rd2 = generate_random_large_number(50);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        cpp_int sum = num1 / num2;

        BigInt big1(rd1);
        BigInt big2(rd2);

        BigInt result = big1 / big2;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }
}

TEST(IntegerTest, SpaceShipTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(50);
        std::string rd2 = generate_random_large_number(50);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        BigInt big1(rd1);
        BigInt big2(rd2);

        EXPECT_EQ(big1 < big2, num1 < num2);
        EXPECT_EQ(big1 <= big2, num1 <= num2);
        EXPECT_EQ(big1 > big2, num1 > num2);
        EXPECT_EQ(big1 >= big2, num1 >= num2);
    }

    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(50);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd1));

        BigInt big1(rd1);
        BigInt big2(rd1);

        EXPECT_EQ(big1 < big2, num1 < num2);
        EXPECT_EQ(big1 <= big2, num1 <= num2);
        EXPECT_EQ(big1 > big2, num1 > num2);
        EXPECT_EQ(big1 >= big2, num1 >= num2);
    }
}