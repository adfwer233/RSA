#pragma once

#include "integer/integer.hpp"
#include "integer/prime_generator.hpp"

/**
 * @brief RSA implementation
 * @tparam IntegerType Biginteger Type
 */
template<typename IntegerType>
struct RSA {
    IntegerType generate_prime(size_t hex_bit_count) {
        auto result = PrimeGenerator<IntegerType>::get_prime(hex_bit_count);
        return result;
    }
};