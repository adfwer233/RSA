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
        std::string rd1 = generate_random_large_number(400);
        std::string rd2 = generate_random_large_number(200);
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

inline int bit_test(const BigInt& value, size_t b) {
    return value.bit_test(b);
}

template<typename IntegerType>
auto mod_exp(IntegerType base, IntegerType exponent, const IntegerType& mod) -> IntegerType {
    IntegerType result{1};
    base = base % mod;
    while (exponent > 0) {
        if (bit_test(exponent, 0)) {
            result = (result * base) % mod;
        }
        exponent >>= 1;

        base = (base * base) % mod;
    }

    return result;
};

TEST(IntegerTest, SingleOperationBenchmark) {
    double boost_sum = 0;
    double our_sum = 0;

    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(30);
        cpp_int num1(convert_hex_to_dec(rd1));

        auto boost_start = std::chrono::steady_clock::now();
        cpp_int sum = num1 % 12345;
        auto boost_end = std::chrono::steady_clock::now();

        double duration_boost = std::chrono::duration<double, std::nano>(boost_end - boost_start).count();
        boost_sum += duration_boost;

        BigInt big1(rd1);

        std::cout << "start" << std::endl;

        auto our_start = std::chrono::steady_clock::now();
        auto result = big1 % 12345;
        auto our_end = std::chrono::steady_clock::now();
        std::cout << "finish" << std::endl;

        our_sum += std::chrono::duration<double, std::nano>(our_end - our_start).count();

        EXPECT_EQ(convert_hex_to_dec(BigInt(result).to_string()), sum.str());
    }

    std::cout << boost_sum << std::endl << our_sum << std::endl;
}

TEST(IntegerTest, ModExpBenchmark) {
    double boost_sum = 0;
    double our_sum = 0;

    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(192);
        std::string rd2 = generate_random_large_number(192);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        auto boost_start = std::chrono::steady_clock::now();
        cpp_int sum = mod_exp(cpp_int{12345}, num1, num2);
        auto boost_end = std::chrono::steady_clock::now();

        double duration_boost = std::chrono::duration<double, std::nano>(boost_end - boost_start).count();
        boost_sum += duration_boost;

        BigInt big1(rd1);
        BigInt big2(rd2);

        auto our_start = std::chrono::steady_clock::now();
        auto result = mod_exp(BigInt {12345}, big1, big2);
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

        if (convert_hex_to_dec(result.to_string()) != sum.str()) {
            std::cout << rd1 << std::endl;
            std::cout << rd2 << std::endl;
        }
    }
}

TEST(IntegerTest, ModTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(1000);
        std::string rd2 = generate_random_large_number(500);
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        cpp_int sum = num1 % num2;

        BigInt big1(rd1);
        BigInt big2(rd2);

        BigInt result = big1 % big2;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }
}

TEST(IntegerTest, ShiftTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(1000);
        cpp_int num1(convert_hex_to_dec(rd1));

        cpp_int sum = num1 >>= 1;

        BigInt big1(rd1);

        BigInt result = big1 >>= 1;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
    }
}

TEST(IntegerTest, SingleMinusTest) {
    for (int i = 0; i < 10; ++i) {
        std::string rd1 = generate_random_large_number(1000);
        cpp_int num1(convert_hex_to_dec(rd1));

        cpp_int sum = num1 - 1;

        BigInt big1(rd1);

        BigInt result = big1 - 1;

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

TEST(IntegerTest, ModTest1) {
    std::string rd1 = "0x1c130ff1327fe487584457e1f5c16bbdd80ab606a5600d5d20";
    std::string rd2 = "0x1205152f6707c1f88495ffe043ed32ac75074905968376f6d6";
    cpp_int num1(convert_hex_to_dec(rd1));
    cpp_int num2(convert_hex_to_dec(rd2));

    cpp_int sum = num1 % num2;

    BigInt big1(rd1);
    BigInt big2(rd2);

    BigInt result = big1 % big2;

    EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
}

TEST(IntegerTest, ModTest2) {
        std::string rd1 = "0x103b5075ca26dcab629327871db769303a0eb8653e0a30c7c05b1c186dc18dc480ab47da08ef67d49c0cdd44f954a5f93b26f9018431073249cf7d70a7d3681b4640cb3397d646c0e29ea0e55bb293f20fbcd209b8206fc6dc23bc05a7ed9e6d2dcad645d7729345d0e6eb211934d0edde4d1998d103f2cf45d1a2abff852e7a8a21581a06a3acfab0477f9c80000000000000000";
        std::string rd2 = "0xf353b8d83730c1556039a1570fc40c94b73c32a8a8d95cbdeadf2120cf7b52a8e3c8e54a9a5899fee7c07478ad8a371bb14e5a1e32912d7f56d82ac1bbdd4747b699894143a6d225d94feac3ea9618629d966de859a580ac5c741b1150275285";
        cpp_int num1(convert_hex_to_dec(rd1));
        cpp_int num2(convert_hex_to_dec(rd2));

        cpp_int sum = num1 % num2;

        BigInt big1(rd1);
        BigInt big2(rd2);

        BigInt result = big1 % big2;

        EXPECT_EQ(convert_hex_to_dec(result.to_string()), sum.str());
}