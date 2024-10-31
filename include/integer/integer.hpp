#pragma once

#include <cmath>
#include <vector>
#include <cstdint>
#include <iomanip>

/**
 * @brief large integer data structure
 *
 * @tparam bit should be times of 4
 */
template<int bit = 32>
struct Integer {
    explicit Integer() = default;

    explicit Integer(int val) {
        from_int(val);
    }

    explicit Integer(const std::string_view value) {
        from_string(value);
    }

    ~Integer() = default;

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
            other.current_length = 0;
        }
        return *this;
    }

    auto operator <=> (const Integer& other) const {
        if (current_length != other.current_length) {
            return current_length <=> other.current_length;
        }

        for (int i = current_length - 1; i >= 0; i--) {
            if (data[i] != other.data[i]) {
                return data[i] <=> other.data[i];
            }
        }

        return std::strong_ordering::equal;
    }

    Integer operator + (const Integer& other) const {
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

    Integer operator - (const Integer& other) const {
        Integer result;
        result.alloc_data(current_length);
        result.current_length = current_length;

        int64_t borrow = 0;
        for (size_t i = 0; i < current_length; ++i) {
            int64_t subtrahend = (i < other.current_length ? other.data[i] : 0) + borrow;
            int64_t difference = static_cast<int64_t>(data[i]) - subtrahend;

            if (difference < 0) {
                difference += radix();
                borrow = 1;
            } else {
                borrow = 0;
            }

            result.data[i] = static_cast<uint64_t>(difference);
        }

        // Remove any leading zeroes in the result
        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
            result.current_length--;
        }

        return result;
    }

    Integer operator + (const uint64_t other) const {
        return add_one_bit(other);
    }

    Integer operator * (const Integer& other) const {
        return long_multiplication(other);
    }

    Integer operator * (const uint64_t other) const {
        return multiply_one_bit(other);
    }

    Integer operator / (const Integer& other) const {
        Integer reminder;
        return long_division(other, reminder);
    }

    void from_int(int val) {
        current_length = 1;
        alloc_data(current_length);
        data[0] = static_cast<uint64_t>(val);
    }

    /**
     * value should in hex type, with 0x prefix
     * @param value
     */
    void from_string(const std::string_view value) {
        current_length = 0;

        auto value_no_prefix = value.substr(2, value.length() - 2);

        size_t tmp = value_no_prefix.length() * 4;
        size_t len = (value_no_prefix.length() * 4 + bit - 1) / bit;
        alloc_data(len);

        for (int i = static_cast<int>(value_no_prefix.size()); i > 0; i -= bit / 4) {
            int start = std::max(0, i - bit / 4);
            int length = i - start;
            std::string part = std::string(value_no_prefix.substr(start, length));

            data[current_length++] = std::stoull(part, nullptr, 16);
        }
    }

    [[nodiscard]] std::string to_string() const {
        std::stringstream ss;

        ss << "0x";

        for (int i = current_length - 1; i >= 0; i--) {
            if (i == current_length - 1) {
                ss << std::hex << data[i];
            } else {
                ss << std::setw(bit / 4) << std::hex << std::setfill('0') << data[i];
            }
        }
        return ss.str();
    }

private:
    Integer add_one_bit(const uint64_t other) const {
        Integer result;
        size_t n = current_length + 1;
        result.current_length = n;
        result.alloc_data(n);

        size_t carry = other;

        for (int i = 0; i <current_length; i++) {
            uint64_t sum = data[i] + carry;
            carry = sum / radix();
            result.data[i] = sum % radix();
        }

        result.data[n - 1] = carry;

        while (result.current_length > 1 and result.data[result.current_length - 1] == 0)
            result.current_length --;

        return result;
    }

    Integer multiply_one_bit(const uint64_t other) const {
        Integer result;
        size_t n = current_length + 1;
        result.current_length = n;
        result.alloc_data(n);

        size_t carry = 0;

        for (int i = 0; i <current_length; i++) {
            uint64_t prod = data[i] * other + carry;
            carry = prod / radix();
            result.data[i] = prod % radix();
        }

        result.data[n - 1] = carry;

        while (result.current_length > 1 and result.data[result.current_length - 1] == 0)
            result.current_length --;

        return result;
    }

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

    Integer long_division(const Integer& divisor, Integer& remainder) const {
        if (divisor.current_length == 1 && divisor.data[0] == 0) {
            throw std::invalid_argument("Division by zero");
        }

        Integer quotient;
        quotient.alloc_data(current_length);  // Maximum possible length for quotient
        quotient.current_length = current_length;

        remainder = Integer();  // Initialize remainder as zero
        remainder.alloc_data(current_length);
        remainder.current_length = 0;

        // Traverse from the most significant chunk to the least significant chunk of `*this`
        for (int i = static_cast<int>(current_length) - 1; i >= 0; --i) {
            // Shift remainder by one base unit and add the next chunk of `data`
            remainder = remainder * radix() + data[i];

            // Determine how many times divisor fits into remainder (trial division)
            uint64_t left = 0, right = radix() - 1, quotient_chunk = 0;
            while (left <= right) {
                uint64_t mid = (left + right) / 2;
                Integer candidate = divisor * mid;
                if (candidate <= remainder) {
                    quotient_chunk = mid;
                    left = mid + 1;
                } else {
                    right = mid - 1;
                }
            }

            if (quotient_chunk != 0) {
                int x = 0;
            }

            // Set quotient at this position and subtract the matched amount from remainder
            quotient.data[i] = quotient_chunk;
            remainder = remainder - divisor * quotient_chunk;
        }

        while (quotient.current_length > 1 && quotient.data[quotient.current_length - 1] == 0) {
            quotient.current_length--;
        }

        return quotient;
    }

    void alloc_data(int len) {
        data.clear();
        data.reserve(len * 2);
        data.resize(len * 2);

        for (int i = 0; i < len; i++) {
            data[i] = 0;
        }
    }

    [[nodiscard]] constexpr size_t radix() const {
        return std::pow(2, bit);
    }

    std::vector<uint64_t> data;
    size_t current_length = 0;
};

using BigInt = Integer<32>;