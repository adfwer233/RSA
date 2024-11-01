#pragma once

#include <random>
#include <string>

struct Random {
    enum class DigitFormat {
        dec, hex
    };

    template<DigitFormat Format = DigitFormat::hex>
    static std::string generate_random_large_number(size_t digits) {
        if constexpr (Format == DigitFormat::hex) {
            static const char hex_chars[] = "0123456789abcdef";
            std::string result;
            result.reserve(digits);

            // Random number generator
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dist(0, 15); // Range for hex characters
            std::uniform_int_distribution<int> dist_non_zero(8, 15); // Range for hex characters

            // Generate each hex digit
            for (size_t i = 0; i < digits; ++i) {
                if (i == 0)
                    result += hex_chars[dist_non_zero(gen)];
                else
                    result += hex_chars[dist(gen)];
            }
            return "0x" + result;
        } else {
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
    }
};