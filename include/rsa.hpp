#pragma once

#include "spdlog/spdlog.h"

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

    struct PublicKey {
        BigInt n;
        BigInt e;
    };

    struct PrivateKey {
        BigInt p;
        BigInt q;
        BigInt n;
        BigInt d;
        BigInt phi;
    };

    /**
     * @brief encrypt the given message (represented in bytes)
     * @param message
     * @return
     */
    BigInt encrypt(const BigInt& message) {
        spdlog::info(message.to_string());
        spdlog::info(public_key.e.to_string());
        spdlog::info(public_key.n.to_string());
        return BigInt::fast_odd_exp_mod(message, public_key.e, private_key.n);
    }

    /**
     * decrypt the cipher
     * @param cipher
     * @return the byte representation of the message
     */
    BigInt decrypt(const BigInt& cipher) {
        return BigInt::fast_odd_exp_mod(cipher, private_key.d, private_key.n);
    }

    /**
     * @brief sign the digest
     * @param digest
     * @return
     */
    BigInt sign(const BigInt& digest) {
        return BigInt::fast_odd_exp_mod(digest, private_key.d, private_key.n);
    }

    /**
     * @brief verify the digest
     * @param digest
     * @param signature
     * @return
     */
    bool verify(const BigInt& digest, const BigInt signature) {
        auto encrypted = BigInt::fast_odd_exp_mod(digest, public_key.e, private_key.n);
        return encrypted == signature;
    }

    /**
     * @biref generate RSA key pair with given lenght
     * @param len should be times of 4
     * @return [public key, private key]
     */
    std::pair<PublicKey, PrivateKey> generate_key_pair(size_t len) {
        BigInt p = generate_prime(len / 4);
        BigInt q = generate_prime(len / 4);
        BigInt n = p * q;
        BigInt phi = (p - 1) * (q - 1);
        BigInt e = choose_e(n);
        BigInt d = mod_inverse(e, phi);
        BigInt t = (e * d);
        BigInt t2 = t % phi;
        std::cout << t2.to_string() << std::endl;
        public_key = {n, e};
        private_key = {p, q, n, d, phi};
        return {public_key, private_key};
    }
//private:

    SignedBigInt exgcd(const SignedBigInt &a, const SignedBigInt &b, SignedBigInt &x, SignedBigInt &y) {
        if (b.abs == BigInt(0)) {
            x = BigInt(1);
            y = BigInt(0);
            return a;
        }
        SignedBigInt x1, y1;
        SignedBigInt gcd = exgcd(b, a % b, x1, y1);
        x = y1;
        y = x1 - (a / b) * y1;
        return gcd;
    }

    BigInt mod_inverse(const BigInt &x, const BigInt &n) {
        SignedBigInt sx = x, sn = n;
        SignedBigInt x_inv, y;
        SignedBigInt gcd = exgcd(sx, sn, x_inv, y);

        if (not(gcd.abs == BigInt(1))) {
            throw std::invalid_argument("Inverse does not exist");
        }

        return ((x_inv % n + n) % n).abs;
    }

    BigInt choose_e(const BigInt& n) {
        return BigInt("0x10001");
    }

    PublicKey public_key;
    PrivateKey private_key;
};