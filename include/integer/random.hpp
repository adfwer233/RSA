#pragma once

#include <random>
#include <string>

struct Random {
    static std::string generate_random_large_number(size_t digits) {
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
};