#include <benchmark/benchmark.h>
#include <array>

#include "rsa.hpp"

constexpr int len = 6;

static void rsa_768_benchmark(benchmark::State& state) {
    RSA<BigInt> rsa_manager;
    for (auto _: state) {
        rsa_manager.generate_key_pair(768);
    }
}

static void rsa_1024_benchmark(benchmark::State& state) {
    RSA<BigInt> rsa_manager;
    for (auto _: state) {
        rsa_manager.generate_key_pair(1024);
    }
}

static void rsa_2048_benchmark(benchmark::State& state) {
    RSA<BigInt> rsa_manager;
    for (auto _: state) {
        rsa_manager.generate_key_pair(2048);
    }
}

static void rsa_4096_benchmark(benchmark::State& state) {
    RSA<BigInt> rsa_manager;
    for (auto _: state) {
        rsa_manager.generate_key_pair(4096);
    }
}

BENCHMARK(rsa_768_benchmark);
BENCHMARK(rsa_1024_benchmark);
BENCHMARK(rsa_2048_benchmark);
BENCHMARK(rsa_4096_benchmark);

BENCHMARK_MAIN();