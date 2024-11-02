#pragma once

#include <cmath>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <sstream>

/**
 * @brief large integer data structure
 *
 * @tparam bit should be times of 32 / 64
 */
template<int bit, typename DataType , typename InterDataType, typename SignedInterDataType>
struct Integer {
    explicit Integer() {
        current_length = 0;
    };

    explicit Integer(int val) {
        from_int(val);
    }

    explicit Integer(DataType val) {
        from_single(val);
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

        return data[0] <=> static_cast<DataType>(other);
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

        DataType carry = 0;
        for (size_t i = 0; i < n; i++) {
            DataType a = i < current_length ? data[i] : 0;
            DataType b = i < other.current_length ? other.data[i]: 0;
            DataType sum = a + b + carry;
            result.data[i] = sum;
            carry = sum >= b ? 0 : 1;
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

        DataType borrow = 0;
        for (size_t i = 0; i < current_length; ++i) {
            DataType subtrahend = (i < other.current_length ? other.data[i] : 0) + borrow;
            DataType difference = data[i] - subtrahend;
            borrow = difference > data[i] ? 1 : 0;
            result.data[i] = static_cast<DataType>(difference);
        }

        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
            result.current_length--;
        }
        return result;
    }

    Integer operator + (const DataType other) const {
        return add_one_bit(other);
    }

    Integer operator - (const DataType other) const {
        Integer value;
        value.alloc_data(1);
        value.current_length = 1;
        value.data[0] = other;

        return *this - value;
    }

    Integer operator * (const Integer& other) const {
        return karatsuba_multiplication(other);
    }

    Integer operator * (const DataType other) const {
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

    DataType operator % (int other) const {
        DataType reminder;
        divide_one_bit(other, reminder);
        return reminder;
    }

    bool operator == (const int other) const {
        if (current_length != 1) return false;
        return data[0] == other;
    }

    bool operator == (const DataType other) const {
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
            DataType carry = 0;
            for (int i = static_cast<int>(current_length) - 1; i >= 0; --i) {
                DataType new_carry = (data[i] << bit_shift_inv) % radix();
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

    template <typename T> int high_bit(T x) const {
        auto ux = std::make_unsigned_t<T>(x);
        int lb = -1, rb = std::numeric_limits<decltype(ux)>::digits;
        while (lb + 1 < rb)
        {
            int mid = (lb + rb) / 2;
            if (ux >> mid)
            {
                lb = mid;
            }
            else
            {
                rb = mid;
            }
        }
        return lb;
    }

    [[nodiscard]] int msb() const {
        return (current_length - 1) * bit + high_bit(data[current_length - 1]) + 1;
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
        data[0] = static_cast<DataType>(val);
    }

    void from_single(DataType val) {
        current_length = 1;
        alloc_data(current_length);
        data[0] = val;
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

    static Integer fast_odd_exp_mod(const Integer& base, const Integer& exp, const Integer& mod) {
        if (not mod.bit_test(0)) {
            throw std::runtime_error("this only for computing exponential of odd numbers");
        }

        DataType r = mod.current_length * bit;
        Integer R = Integer{1}.left_shift_chunk(mod.current_length);
        Integer mod_inverse = mod.inverse_mod_2_pow(r);

        Integer result = montgomery_transformation(Integer(1), mod, r);
        Integer a = montgomery_transformation(base, mod, r);

        Integer exp_prime = exp;

        while (exp_prime > 0) {
            if (exp_prime.bit_test(0)) {
                result = montgomery_multiplication(result, a, mod, mod_inverse, R, r);
            }
            exp_prime >>= 1;
            a = montgomery_multiplication(a, a, mod, mod_inverse, R, r);
        }

        return montgomery_reduce(result, R, r, mod, mod_inverse);
    }

    //private:
    Integer add_one_bit(const DataType other) const {
        Integer result;
        size_t n = current_length + 1;
        result.current_length = n;
        result.alloc_data(n);

        DataType carry = other;

        for (int i = 0; i <current_length; i++) {
            DataType sum = data[i] + carry;
            carry = 0;
            if (sum < data[i]) carry = 1;
            result.data[i] = sum;
        }

        result.data[n - 1] = carry;

        while (result.current_length > 1 and result.data[result.current_length - 1] == 0)
            result.current_length --;

        return result;
    }

    Integer multiply_one_bit(const DataType other) const {
        Integer result;
        size_t n = current_length + 1;
        result.current_length = n;
        result.alloc_data(n);

        size_t carry = 0;

        for (int i = 0; i < current_length; i++) {
            InterDataType prod = static_cast<InterDataType>(data[i]) * static_cast<InterDataType>(other) + carry;
            carry = prod / radix();
            result.data[i] = prod % radix();
        }

        result.data[n - 1] = carry;

        while (result.current_length > 1 and result.data[result.current_length - 1] == 0)
            result.current_length --;

        return result;
    }

    Integer divide_one_bit(const DataType divisor, DataType& reminder) const {
        if (divisor > radix())
            throw std::runtime_error("divisor too large, use Integer");

        int n = current_length;
        Integer result;
        result.alloc_data(n);
        result.current_length = n;

        InterDataType cur_reminder = 0;
        for (int i = n - 1; i >= 0; i--) {
            cur_reminder = static_cast<InterDataType>(cur_reminder << bit) + data[i];

            InterDataType quotient = cur_reminder / divisor;
            cur_reminder -= quotient * divisor;

            result.data[i] = quotient % radix();
            result.data[i + 1] += quotient >> bit;
        }

        result.remove_leading_zero();
        reminder = cur_reminder;
        return std::move(result);
    }

    void subtract_inplace(const Integer& other) {
        DataType borrow = 0;
        for (size_t i = 0; i < current_length; ++i) {
            DataType subtrahend = other.data[i] + borrow;
            DataType difference = data[i] - subtrahend;

            borrow = difference > data[i] ? 1 : 0;

            data[i] = static_cast<DataType>(difference);
        }

        remove_leading_zero();
    }

    Integer long_multiplication(const Integer& other) const {
        Integer result;

        size_t n = current_length + other.current_length;
        result.current_length = current_length + other.current_length;
        result.alloc_data(n);

        for (size_t j = 0; j < other.current_length; j++) {
            DataType carry = 0;
            for (int i = 0; i < current_length ; i += 2) {
                InterDataType prod = static_cast<InterDataType>(other.data[j]) * static_cast<InterDataType>(data[i]) + result.data[i + j] + carry;
                carry = static_cast<DataType>(prod >> bit);
                result.data[i + j] = static_cast<DataType>(prod); // @todo: only works when bit = 32

                if (i + 1 < current_length) {
                    InterDataType prod2 = static_cast<InterDataType>(other.data[j]) * static_cast<InterDataType>(data[i + 1]) + result.data[i + j + 1] + carry;
                    carry = static_cast<DataType>(prod2 >> bit);
                    result.data[i + j + 1] = static_cast<DataType>(prod2);
                }
            }
            result.data[current_length + j] = carry;
        }

        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
            result.current_length--;
        }

        return result;
    }

    Integer karatsuba_multiplication(const Integer& other) const {
        // Base case: use long multiplication for small numbers
        if (current_length <= 128 || other.current_length <= 128) {
            return long_multiplication(other);
        }
        size_t n = std::max(current_length, other.current_length);
        size_t half = (n + 1) / 2;
        bool res = *this < other;
        const Integer & v1 = res ? other: *this;
        const Integer & v2 = res ? *this: other;
        // Split `this` into high and low parts
        Integer low1, high1;
        low1.data = std::vector<DataType>(v1.data.begin(), v1.data.begin() + half);
        high1.data = std::vector<DataType>(v1.data.begin() + half, v1.data.begin() + v1.current_length);
        low1.current_length = half;
        high1.current_length = v1.current_length - half;
        Integer result;
        if (v2.current_length <= half) {
            Integer z0 = std::move(high1.karatsuba_multiplication(v2));
            Integer z1 = std::move(low1.karatsuba_multiplication(v2));
            result = std::move(z0.left_shift_chunk(half) + z1);
        } else {
            // Split `other` into high and low parts
            Integer low2, high2;
            low2.data = std::vector<DataType>(v2.data.begin(), v2.data.begin() + half);
            high2.data = std::vector<DataType>(v2.data.begin() + half, v2.data.begin() + v2.current_length);
            low2.current_length = low2.data.size();
            high2.current_length = high2.data.size();
            // Recursively calculate three products
            Integer z0 = std::move(low1.karatsuba_multiplication(low2));
            Integer z2 = std::move(high1.karatsuba_multiplication(high2));
            Integer z1 = std::move((low1 + high1).karatsuba_multiplication(low2 + high2) - z0 - z2);
            result = std::move(z0 + z1.left_shift_chunk(half) + z2.left_shift_chunk(half * 2));
        }
        // Update result length
        result.current_length = current_length + other.current_length;
        while (result.current_length > 1 && result.data[result.current_length - 1] == 0) {
            --result.current_length;
        }
        return result;
    }

    void remove_leading_zero() {
        while(current_length > 1 && data[current_length - 1] == 0) current_length --;
    }

    Integer left_shift_chunk(size_t chunk_count) const {
        Integer result;
        result.alloc_data(chunk_count + current_length);
        result.current_length = current_length + chunk_count;

        for (int i = 0; i < chunk_count; i++) result.data[i] = 0;

        std::copy(data.begin(), data.begin() + current_length, result.data.begin() + chunk_count);
        return result;
    }

    Integer right_shift_chunk(size_t chunk_count) const {
        return get_chunks(chunk_count, current_length - chunk_count);
    }

    Integer mod_2_pow(size_t k) const {
        if (k % bit != 0) {
            throw std::runtime_error("only support module 2 ^ {n * bit} for efficiency");
        }
        int chunks = k / bit;
        Integer result;
        result.alloc_data(chunks);
        std::copy(data.begin(), data.begin() + chunks, result.data.begin());
        result.current_length = chunks;
        return result;
    }

    /**
     * @brief compute the inverse of *this w.r.t. 2^k, and *this should be odd
     * @param k
     * @return
     */
    Integer inverse_mod_2_pow(size_t k) const {
        Integer result{1};
        Integer base = *this;
        for (int i = 0; i < k - 1; i ++) {
            result = (result * base).mod_2_pow(k);
            base = (base * base).mod_2_pow(k);
        }
        return result;
    }

    static Integer montgomery_multiplication(const Integer& a, const Integer& b, const Integer& mod, const Integer& mod_inverse, const Integer& R, uint64_t r) {
        Integer c = a * b;
        return montgomery_reduce(c, R, r, mod, mod_inverse);
    }

    static Integer montgomery_reduce(const Integer& x, const Integer& R, uint64_t r, const Integer& mod, const Integer& mod_inverse) {
        Integer q = (x.mod_2_pow(r) * (R - mod_inverse)).mod_2_pow(r);
        Integer a = x + q * mod;
        a = a.right_shift_chunk(r / bit);
        if (a >= mod) {
            a = a - mod;
        }
        return a;
    }

    static Integer montgomery_transformation(const Integer& x, const Integer& mod, uint64_t r) {
        Integer x_re = x.left_shift_chunk(r / bit);
        return x_re % mod;
    }

    Integer get_chunks(size_t start, size_t length) const {
        Integer result;
        result.alloc_data(length);
        result.current_length = length;

        std::copy(data.begin() + start, data.begin() + start + length, result.data.begin());
        return result;
    }

    Integer knuth_division(const Integer& t_divisor, Integer& t_reminder) const {
        if (*this < t_divisor) {
            t_reminder = *this + 0;
            return zero();
        }

        Integer dividend = *this;
        Integer divisor = t_divisor;
        Integer result;
        DataType v = radix() / (divisor.data[divisor.current_length - 1] + 1);
        dividend = dividend * v;
        divisor = divisor * v;

        int n = dividend.current_length;
        int m = divisor.current_length;
        result.alloc_data(n - m);

        DataType highest  = divisor.data[divisor.current_length - 1];

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

        if (dividend.data[n - 1] == 0) {
            std::cout << "wrong" << std::endl;
        }

        for (int i = n - m - 1; i >= 0; i--) {
            Integer reminder = dividend.get_chunks(i, 1 + m);

            // quotient estimation
            SignedInterDataType q = (static_cast<InterDataType>(reminder.data[m]) * radix() + reminder.data[m - 1]) / static_cast<InterDataType>(highest) - 2;
            q = std::max(q, static_cast<SignedInterDataType>(0));

            // remove leading zeros
            reminder.remove_leading_zero();
            if (q < radix()) {
                reminder.subtract_inplace(divisor.multiply_one_bit(q));
            } else {
                DataType q_low = q % radix();
                DataType q_high = q >> bit;
                reminder.subtract_inplace(divisor.multiply_one_bit(q_low) + divisor.left_shift_chunk(1).multiply_one_bit(q_high));
            }

            int t = 0;
            while (reminder >= divisor) {
                q++;
                t++;
                if (t > 3) {
                    throw std::runtime_error("knuth division failed");
                }
                reminder.subtract_inplace(divisor);
            }

            std::copy(reminder.data.begin(), reminder.data.begin() + reminder.current_length, dividend.data.begin() + i);
            std::fill(dividend.data.begin() + i + reminder.current_length, dividend.data.begin() + i + m + 1, 0);

            result.data[i] = q % radix();
            result.data[i + 1] += q >> bit;
        }

        result.current_length = n - m + 1;
        while(result.current_length >= 1 and result.data[result.current_length - 1] == 0) result.current_length--;

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
            InterDataType left = 0, right = radix() - 1, quotient_chunk = 0;
            while (left <= right) {
                InterDataType mid = (left + right) / 2;
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
        data.resize(len + 2);
        std::fill(data.begin(), data.begin() + len, 0);
    }

    [[nodiscard]] constexpr InterDataType radix() const {
        return std::pow(2, bit);
    }

    std::vector<DataType> data;
    size_t current_length = 0;
};

#if defined(__GNUC__)
using BigInt = Integer<64, uint64_t, __uint128_t, __int128_t>;
#else
using BigInt = Integer<32, uint32_t, uint64_t, int64_t>;
#endif
