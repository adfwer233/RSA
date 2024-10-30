#pragma once

#include "random.hpp"

template<typename IntegerType>
struct PrimeGenerator {

    static IntegerType generate_random(const IntegerType& min, const IntegerType& max) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());

        IntegerType range = max - min;
        IntegerType result = min;

        // Generate the number in chunks
        do {
            result = min;
            std::cout << result << std::endl;
            for (size_t bits = msb(range) + 1; bits > 0;) {
                size_t chunk_size = std::min<size_t>(bits, 64);
                std::uniform_int_distribution<uint64_t> dist(0, (1ULL << chunk_size) - 1);
                result = (result << chunk_size) | dist(gen);
                bits -= chunk_size;
            }
        } while (result > max);

        return result;
    }

    static bool pass_miller_rabin(const IntegerType& value, int iterations = 5) {
        if (value < 2) return false;
        if (value == 2 || value == 3) return true;
        if (bit_test(value, 0) == 0) return false;  // Even numbers > 2 are not prime

        IntegerType d = value - 1;
        int s = 0;

        // Write value - 1 as d * 2^s by factoring out powers of 2 from d
        while (bit_test(d, 0) == 0) {
            d >>= 1;
            ++s;
        }

        // Helper lambda to perform modular exponentiation
        auto mod_exp = [](IntegerType base, IntegerType exponent, const IntegerType& mod) -> IntegerType {
            IntegerType result = 1;
            base %= mod;
            while (exponent > 0) {
                if (bit_test(exponent, 0)) {
                    result = (result * base) % mod;
                }
                exponent >>= 1;
                base = (base * base) % mod;
            }
            return result;
        };

        // Perform the Miller-Rabin test with the specified number of iterations
        for (int i = 0; i < iterations; ++i) {
            IntegerType a = generate_random(2, value - 2);
            std::cout << i << std::endl;
            IntegerType x = mod_exp(a, d, value);

            if (x == 1 || x == value - 1) continue;

            bool found = false;
            for (int r = 1; r < s; ++r) {
                x = mod_exp(x, 2, value);  // Square x mod value
                if (x == value - 1) {
                    found = true;
                    break;
                }
            }

            if (!found) return false;  // Composite if no witness found
        }

        return true;  // Likely prime if all iterations passed
    }

    /**
     * @brief judge if a integer is prime
     *
     * we use miller-rabin here, try_time ref to
     *
     * @param value
     * @return
     */
    static bool is_prime(IntegerType& value) {
        int bit_length = msb(value);

        int try_time = 0;

        if (bit_length < 100)
            try_time = 50;

        if (bit_length < 256) {
            try_time = 27;
        } else if (bit_length < 512) {
            try_time = 15;
        } else if (bit_length < 768) {
            try_time = 8;
        } else if (bit_length < 1024) {
            try_time = 4;
        } else {
            try_time = 2;
        }

        return pass_miller_rabin(value, try_time);
    }

    /**
     * @brief generate a prime integer with given bit count
     * @param bit_count
     * @return
     */
    static IntegerType get_prime(int bit_count) {
        std::string num_str = Random::generate_random_large_number(bit_count);
        IntegerType value(num_str);

        if (not bit_test(value, 0)) {
            bit_set(value, 0);
        }

        while(not is_prime(value)) {
            value = value + 2;
        }

        return value;
    }

};