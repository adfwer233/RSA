#pragma once

#include <cmath>
#include <vector>
#include <cstdint>
#include <iomanip>

/**
 * @brief large integer data structure
 */
template<int bit = 8>
struct Integer {
    explicit Integer() = default;

    explicit Integer(int val) {
        from_int(val);
    }

    explicit Integer(const std::string_view value) {
        from_string(value);
    }

    ~Integer() {}

    Integer(const Integer& other) : current_length(other.current_length) {
        alloc_data(current_length);
        data = other.data;
    }

    Integer& operator=(const Integer& other) {
        if (this != &other) {
            current_length = other.current_length;
            alloc_data(current_length);
            data = other.data;
        }
        return *this;
    }

    Integer(Integer&& other) noexcept
            : current_length(other.current_length), data(std::move(other.data)) {
        other.current_length = 0;
    }

    Integer& operator=(Integer&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            current_length = other.current_length;
            other.data = nullptr;
            other.current_length = 0;
        }
        return *this;
    }

    Integer operator+(const Integer& other) const {
        Integer result;

        size_t n = std::max(current_length, other.current_length);

        result.alloc_data(std::max(current_length, other.current_length));

        size_t carry = 0;
        for (size_t i = 0; i < n; i++) {
            size_t a = i < current_length ? data[i] : 0;
            size_t b = i < other.current_length ? other.data[i]: 0;
            size_t sum = a + b + carry;
            result.data[i] = sum % radix();
            carry = sum / radix();
        }

        result.current_length = n;

        if (carry > 0) {
            result.data[n] = carry;
            result.current_length = n + 1;
        }

        return result;
    }

    Integer operator*(const Integer& other) const {
        return long_multiplication(other);
    }

    Integer operator / (const Integer& other) const {
        return long_divide(other);
    }

    void from_int(int val) {
        current_length = 1;
        alloc_data(current_length);
        data[0] = static_cast<uint64_t>(val);
    }

    void from_string(const std::string_view value) {
        current_length = 0;

        size_t len = (value.length() + bit - 1) / bit;
        alloc_data(len);

        for (int i = static_cast<int>(value.size()); i > 0; i -= bit) {
            int start = std::max(0, i - bit);
            int length = i - start;
            std::string part = std::string(value.substr(start, length));

            data[current_length++] = std::stoull(part);
        }
    }

    size_t get_bit_length() const {
        return bit;
    }

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;
        for (int i = current_length - 1; i >= 0; i--) {
            if (i == current_length - 1) {
                ss << data[i];
            } else {
                ss << std::setw(bit) << std::setfill('0') << data[i];
            }
            // std::cout << data[i] << std::endl;
        }
        return ss.str();
    }

private:
    Integer long_multiplication(const Integer& other) const {
        Integer result;

        size_t n = current_length * other.current_length;
        result.current_length = current_length + other.current_length;
        result.alloc_data(n);

        size_t carry = 0;

        for (int i = 0; i < n; i++) {
            result.data[i] = 0;
        }

        for (int i = 0; i < current_length; i++) {
            for (size_t j = 0; j < other.current_length || carry > 0; ++j) {
                uint64_t b = (j < other.current_length) ? other.data[j] : 0;
                uint64_t prod = data[i] * b + result.data[i + j] + carry;
                carry = prod / radix();
                result.data[i + j] = prod % radix();
            }
        }

        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
            result.current_length--;
        }

        return result;
    }

    void alloc_data(int len) {
        data.clear();
        data.reserve(len * 2);

        for (int i = 0; i < len; i++) {
            data[i] = 0;
        }
    }

    [[nodiscard]] constexpr size_t radix() const {
        return std::pow(10, bit);
    }

    std::vector<uint64_t> data;
    size_t current_length = 0;
};

using BigInt = Integer<8>;