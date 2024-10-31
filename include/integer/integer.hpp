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

    Integer& operator=(const int other) {
        Integer value;
        value.alloc_data(1);
        value.current_length = 1;
        value.data[0] = other;
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

    auto operator <=> (const int other) const {
        if (current_length > 1) {
            return std::strong_ordering::greater;
        }

        if (current_length == 0) {
            return std::strong_ordering::less;
        }

        return data[0] <=> static_cast<uint64_t>(other);
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

    bool operator==(const Integer& rhs) {
        if (current_length != rhs.current_length) {
            return false;
        }

        for (size_t i = 0; i < current_length; ++i) {
            if (data[i] != rhs.data[i]) {
                return false;
            }
        }
        return true;
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
            result.data[i] = sum & 0xFFFFFFFF;
            carry = sum >> bit;
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

    Integer operator - (const uint64_t other) const {
        Integer value;
        value.alloc_data(1);
        value.current_length = 1;
        value.data[0] = other;

        return *this - value;
    }

    Integer operator * (const Integer& other) const {
        return long_multiplication(other);
    }

    Integer operator * (const uint64_t other) const {
        return multiply_one_bit(other);
    }

    Integer operator / (const Integer& other) const {
        Integer reminder;
        return knuth_division(other, reminder);
    }

    Integer operator % (const Integer& other) const {
        Integer reminder;
        knuth_division(other, reminder);
        return reminder;
    }

    Integer operator % (int other) const {
        Integer reminder;
        Integer divisor;
        divisor.alloc_data(1);
        divisor.current_length = 1;
        divisor.data[0] = other;
        knuth_division(divisor, reminder);
        return reminder;
    }

    bool operator == (const int other) const {
        if (current_length != 1) return false;
        return data[0] == other;
    }

    Integer& operator>>=(int shift) {
        if (shift == 0) return *this;

        if (shift > bit)
            throw std::runtime_error("shift value greater than chunk bit");
        int bit_shift = shift % bit;
        int bit_shift_inv = bit - bit_shift;


        if (bit_shift > 0) {
            uint64_t carry = 0;
            for (int i = static_cast<int>(current_length) - 1; i >= 0; --i) {
                uint64_t new_carry = (data[i] << bit_shift_inv) % radix();
                data[i] = (data[i] >> bit_shift) | carry;
                carry = new_carry;
            }
        }

        // Trim any leading zero chunks
        while (current_length > 1 && data[current_length - 1] == 0) {
            --current_length;
        }

        return *this;
    }

    template <typename T> int high_bit(T x) const
    {
        using UT = std::make_unsigned_t<T>;
        return std::numeric_limits<UT>::digits - std::countl_zero(UT(x)) - 1;
    }

    int msb() const {
        return (current_length - 1) * 4 + high_bit<uint64_t>(data[current_length - 1]);
    }

    [[nodiscard]] int bit_test(size_t b) const {
        if (data.empty()) {
            throw std::runtime_error("data empty in bit_test");
        }

        if (b == 0) {
            int tmp = data[0] & 1;
            return tmp;
        }

        return data[b / 4] & (1 << (b - (b / 4) * 4 - 1));
    }

    void bit_set(size_t b) {
        if (data.empty()) {
            throw std::runtime_error("data empty in bit_set");
        }
        if (b == 0) {
            data[0] |= 1;
            return;
        }

        data[b / 4] |= (1 << (b - (b / 4) * 4 - 1));
    }

    Integer zero() const {
        Integer value;
        value.alloc_data(1);
        value.current_length = 0;
        value.data[0] = 0;
        return value;
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

//private:
    Integer add_one_bit(const uint64_t other) const {
        Integer result;
        size_t n = current_length + 1;
        result.current_length = n;
        result.alloc_data(n);

        size_t carry = other;

        for (int i = 0; i <current_length; i++) {
            uint64_t sum = data[i] + carry;
            carry = sum >> bit;
            result.data[i] = sum & 0xFFFFFFFF;
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

        size_t n = current_length + other.current_length;
        result.current_length = current_length + other.current_length;
        result.alloc_data(n);
        size_t carry = 0;

        for (size_t j = 0; j < other.current_length; j++) {
            for (int i = 0; i < current_length ; i += 2) {
                uint64_t prod = other.data[j] * data[i] + result.data[i + j] + carry;
                carry = prod >> bit;
                result.data[i + j] = prod & 0xFFFFFFFF; // @todo: only works when bit = 32

                if (i + 1 < current_length) {
                    uint64_t prod2 = other.data[j] * data[i + 1] + result.data[i + j + 1] + carry;
                    carry = (prod2 >> 32);
                    result.data[i + j + 1] = prod2 & 0xFFFFFFFF;
                }
            }
            if (carry > 0) {
                uint64_t sum = result.data[current_length + j] + carry;
                result.data[current_length + j] = sum & 0xFFFFFFFF;
                carry = sum >> bit;
            }
        }

        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
            result.current_length--;
        }

        return result;
    }

    void remove_leading_zero() {
        while(data[current_length - 1] == 0) current_length --;
    }

    Integer left_shift_chunk(size_t chunk_count) const {
        Integer result;
        result.alloc_data(chunk_count + current_length);
        result.current_length = current_length + chunk_count;

        for (int i = 0; i < chunk_count; i++) result.data[i] = 0;

        std::copy(data.begin(), data.begin() + current_length, result.data.begin() + chunk_count);
        return result;
    }

    Integer get_chunks(size_t start, size_t length) const {
        Integer result;
        result.alloc_data(length);
        result.current_length = length;

        std::copy(data.begin() + start, data.begin() + start + length, result.data.begin());
        return result;
    }

//    Integer karatsuba_multiplication(const Integer& other) const {
//        // Base case: use long multiplication for small numbers
//        if (current_length <= 256 || other.current_length <= 256) {
//            return long_multiplication(other);
//        }
//
//        size_t n = std::max(current_length, other.current_length);
//        size_t half = (n + 1) / 2;
//
//        bool res = *this < other;
//
//        const Integer & v1 = res ? other: *this;
//        const Integer & v2 = res ? *this: other;
//
//        // Split `this` into high and low parts
//        Integer low1, high1;
//        low1.data = std::vector<uint64_t>(v1.data.begin(), v1.data.begin() + half);
//        high1.data = std::vector<uint64_t>(v1.data.begin() + half, v1.data.begin() + v1.current_length);
//        low1.current_length = half;
//        high1.current_length = v1.current_length - half;
//
//        Integer result;
//        if (v2.current_length <= half) {
//            Integer z0 = std::move(high1.karatsuba_multiplication(v2));
//            Integer z1 = std::move(low1.karatsuba_multiplication(v2));
//
//            result = std::move(z0.left_shift_chunk(half) + z1);
//        } else {
//            // Split `other` into high and low parts
//            Integer low2, high2;
//            low2.data = std::vector<uint64_t>(v2.data.begin(), v2.data.begin() + half);
//            high2.data = std::vector<uint64_t>(v2.data.begin() + half, v2.data.begin() + v2.current_length);
//            low2.current_length = low2.data.size();
//            high2.current_length = high2.data.size();
//
//            // Recursively calculate three products
//            Integer z0 = std::move(low1.karatsuba_multiplication(low2));
//            Integer z2 = std::move(high1.karatsuba_multiplication(high2));
//            Integer z1 = std::move((low1 + high1).karatsuba_multiplication(low2 + high2) - z0 - z2);
//            result = std::move(z0 + z1.left_shift_chunk(half) + z2.left_shift_chunk(half * 2));
//        }
//
//        // Update result length
//        result.current_length = current_length + other.current_length;
//        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
//            --result.current_length;
//        }
//
//        return result;
//    }

    Integer knuth_division(const Integer& t_divisor, Integer& t_reminder) const {
        if (*this < t_divisor) {
            t_reminder = *this + 0;
            return zero();
        }

        Integer dividend = *this;
        Integer divisor = t_divisor;
        Integer result;
        uint64_t v = radix() / (divisor.data[divisor.current_length - 1] + 1);
        dividend = dividend * v;
        divisor = divisor * v;

        Integer backup_dividend = dividend;

        int n = dividend.current_length;
        int m = divisor.current_length;
        result.alloc_data(n - m);

        uint64_t highest  = divisor.data[divisor.current_length - 1];

        if (m == n) {
            result.alloc_data(1);
            if (dividend >= divisor) {
                result.current_length = 1;
                result.data[0] = 1;
            } else {
                result.current_length = 0;
                result.data[0] = 0;
            }
            return result;
        }

        if (highest * 2 < radix()) {
            std::cout << "wrong" << std::endl;
        }

        int k = 0;
        for (int i = n - m - 1; i >= 0; i--) {
            Integer current_part = dividend.get_chunks(i, 1 + m);

            // quotient estimation
            long long q = (current_part.data[m] * radix() + current_part.data[m - 1]) / highest - 2;

            q = std::max(q, 1ll);

            // remove leading zeros
            current_part.remove_leading_zero();
            Integer tmp;

            if (q < radix()) {
                tmp = divisor * q;
            } else {
                uint64_t q_low = q % radix();
                uint64_t q_high = q / radix();

                tmp = divisor * q_low + divisor.left_shift_chunk(1) * q_high;
            }

            Integer reminder = current_part - tmp;
            reminder.remove_leading_zero();

            int t = 0;
            while (reminder >= divisor) {
                q++;
                t++;
                if (t > 3) {
                    std::cout << t << std::endl;
                }
                reminder = reminder - divisor;
            }

            std::copy(reminder.data.begin(), reminder.data.begin() + reminder.current_length, dividend.data.begin() + i);

            for (size_t j = i + reminder.current_length; j < i + m + 1; j++) dividend.data[j] = 0;

            result.data[i] = q % radix();
            result.data[i + 1] += q >> bit;

            k++;
        }

        result.current_length = n - m + 1;
        while(result.data[result.current_length - 1] == 0) result.current_length--;

        t_reminder = *this - result * t_divisor;
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
        data.reserve(len +10);
        data.resize(len+10);

        if (len + 10 > 4096 || len + 10 <= 0) {
            std::cout << "test" << std::endl;
        }

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