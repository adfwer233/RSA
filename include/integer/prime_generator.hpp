#pragma once

#include "random.hpp"

template<typename IntegerType>
struct PrimeGenerator {

    static inline std::vector<uint32_t> small_primes = {};

    static IntegerType generate_random(const IntegerType& min, const IntegerType& max) {
        static std::random_device rd;
        static std::mt19937_64 gen(rd());

        std::uniform_int_distribution<uint64_t> dist(0, 32768);

//        IntegerType range = max - min;
//        IntegerType result = min + (max - min) * dist(gen) / std::numeric_limits<uint64_t>::max();

        return 2 + dist(gen);
    }

    static std::vector<uint32_t> generate_primes(int count) {
        int limit = 50000;  // Increase if needed
        std::vector<bool> is_prime(limit, true);
        std::vector<uint32_t> primes;

        // Start marking from the first prime, 2
        for (int i = 2; i < limit && primes.size() < count; ++i) {
            if (is_prime[i]) {
                primes.push_back(i);  // Add i to list of primes
                // Mark multiples of i as non-prime
                for (int j = i * 2; j < limit; j += i) {
                    is_prime[j] = false;
                }
            }
        }

        return primes;
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

        for (auto p: small_primes) {
            if (value % p == 0)
                return false;
        }

        return pass_miller_rabin(value, try_time);
    }

    /**
     * @brief generate a prime integer with given bit count
     * @param bit_count
     * @return
     */
    static IntegerType get_prime(int bit_count) {
        if (small_primes.empty())
            small_primes = generate_primes(4096);

        std::cout << small_primes.size() << std::endl;

        std::string num_str = Random::generate_random_large_number(bit_count);
        IntegerType value(num_str);

        std::cout << msb(value) << std::endl;

        if (not bit_test(value, 0)) {
            bit_set(value, 0);
        }

        int try_num = 0;
        while(not is_prime(value)) {
            try_num ++;
            value = value + 2;
        }

        return value;
    }

};