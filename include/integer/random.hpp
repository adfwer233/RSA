#pragma once

#include <random>
#include <string>

struct Random {
    enum class DigitFormat {
        dec, hex
    };

    template<DigitFormat Format = DigitFormat::hex>
    static std::string generate_random_large_number(size_t digits) {
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
};