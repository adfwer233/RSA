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

    explicit Integer(uint64_t val) {
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

    uint64_t operator % (int other) const {
        uint64_t reminder;
        divide_one_bit(other, reminder);
        return reminder;
    }

    bool operator == (const int other) const {
        if (current_length != 1) return false;
        return data[0] == other;
    }

    bool operator == (const uint64_t other) const {
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
        data[0] = static_cast<uint64_t>(val);
    }

    void from_single(uint64_t val) {
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

        uint64_t r = mod.current_length * bit;
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

    Integer divide_one_bit(const uint64_t divisor, uint64_t& reminder) const {
        if (divisor > radix())
            throw std::runtime_error("divisor too large, use Integer");

        int n = current_length;
        Integer result;
        result.alloc_data(n);
        result.current_length = n;

        uint64_t cur_reminder = 0;
        for (int i = n - 1; i >= 0; i--) {
            cur_reminder = (cur_reminder << bit) + data[i];

            uint64_t quotient = cur_reminder / divisor;
            cur_reminder -= quotient * divisor;

            result.data[i] = quotient & 0xFFFFFFFF;
            result.data[i + 1] += quotient >> bit;
        }

        result.remove_leading_zero();
        reminder = cur_reminder;
        return std::move(result);
    }

    void subtract_inplace(const Integer& other) {
        int64_t borrow = 0;
        for (size_t i = 0; i < current_length; ++i) {
            int64_t subtrahend = other.data[i] + borrow;
            int64_t difference = static_cast<int64_t>(data[i]) - subtrahend;

            if (difference < 0) {
                difference += radix();
                borrow = 1;
            } else {
                borrow = 0;
            }

            data[i] = static_cast<uint64_t>(difference);
        }

        remove_leading_zero();
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
            Integer tmp = (base * base).mod_2_pow(k);
            if (tmp.data[0] == 0) {
                int t = 0;
            }
            base = tmp;
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
        uint64_t v = radix() / (divisor.data[divisor.current_length - 1] + 1);
        dividend = dividend * v;
        divisor = divisor * v;

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

        if (dividend.data[n - 1] == 0) {
            std::cout << "wrong" << std::endl;
        }

        for (int i = n - m - 1; i >= 0; i--) {
            Integer reminder = dividend.get_chunks(i, 1 + m);

            // quotient estimation
            long long q = (reminder.data[m] * radix() + reminder.data[m - 1]) / highest - 2;
            uint64_t t1 = (reminder.data[m] * radix() + reminder.data[m - 1]) / highest;
            q = std::max(q, 0ll);

            // remove leading zeros
            reminder.remove_leading_zero();
            if (q < radix()) {
                reminder.subtract_inplace(divisor.multiply_one_bit(q));
            } else {
                uint64_t q_low = q % radix();
                uint64_t q_high = q / radix();
                reminder.subtract_inplace(divisor.multiply_one_bit(q_low) + divisor.left_shift_chunk(1).multiply_one_bit(q_high));
            }

            int t = 0;
            while (reminder >= divisor) {
                q++;
                t++;
                if (t > 3) {
                    std::cout << (reminder - divisor).to_string() << std::endl;
                    std::cout << radix() << std::endl;
                    std::cout << (reminder.data[m] * radix() + reminder.data[m - 1]) / highest << std::endl;
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
        data.resize(len + 2);
        std::fill(data.begin(), data.begin() + len, 0);
    }

    [[nodiscard]] constexpr size_t radix() const {
        return std::pow(2, bit);
    }

    std::vector<uint64_t> data;
    size_t current_length = 0;
};

using BigInt = Integer<32>;