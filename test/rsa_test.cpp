#include "gtest/gtest.h"

#include "spdlog/spdlog.h"

#include "rsa.hpp"

TEST(RSATest, EncryptAndDecrypt) {
//    BigInt a(15);
//    BigInt b(5);
//
//    EXPECT_TRUE(a % b == BigInt(0));

    RSA<BigInt> rsa_manager;
    rsa_manager.generate_key_pair(768);
    BigInt a("0x20536f6d652054657874204865726520");

    BigInt cipher = rsa_manager.encrypt(a);
    BigInt decrypted = rsa_manager.decrypt(cipher);

    auto ed = rsa_manager.private_key.d * rsa_manager.public_key.e;

    auto tmp = BigInt::fast_odd_exp_mod(BigInt(123), ed, rsa_manager.public_key.n);
    auto tmp2 = BigInt::fast_odd_exp_mod(a, ed, rsa_manager.public_key.n);

    spdlog::info(ed.to_string());
    spdlog::info(tmp.to_string());
    spdlog::info(tmp2.to_string());

    EXPECT_EQ(decrypted.to_string(), a.to_string());
}