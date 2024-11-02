# Simple RSA Implementation

## Introduction

This is a simple implementation of RSA for the applied cryptography course in School of Software, Tsinghua University 

## Features

- Big Interger
  - 2^64 base with GCC (__uint128_t) and 2^32 base for other compiler
  - Algorithms:
    - Karastruba Multiplication
    - Knuth Division
    - Motegomery Multiplication accelerated fast exponential
- RSA
  - Parallelized large prime generator
  - RSA encryption and decryption
  - Digest signature and verification

## Performance

- < 0.1s for RSA-1024 key-pair generation

## Build & Run

- Build the C++ lib

```
mkdir build
cd build && cmake ..
cmake --build .
```

- Run the demo
```
pip install fastapi uvicorn jinja2
cd demo
uvicorn app:app --reload
```
