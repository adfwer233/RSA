#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <iostream>
#include "integer/integer.hpp"
#include "rsa.hpp"

namespace py = pybind11;
PYBIND11_MODULE(rsa_py, variable)
{
    py::class_<BigInt>(variable, "BigInt")
        .def(py::init<const std::string&>())
        .def("to_string", &BigInt::to_string);

    using RSA = RSA<BigInt>;

    py::class_<RSA::PublicKey>(variable, "PublicKey")
            .def_readonly("n", &RSA::PublicKey::n)
            .def_readonly("e", &RSA::PublicKey::e);


    py::class_<RSA::PrivateKey>(variable, "PrivateKey")
            .def_readonly("p", &RSA::PrivateKey::p)
            .def_readonly("q", &RSA::PrivateKey::q)
            .def_readonly("n", &RSA::PrivateKey::n)
            .def_readonly("d", &RSA::PrivateKey::d)
            .def_readonly("phi", &RSA::PrivateKey::phi);

    py::class_<RSA>(variable, "RSA")
        .def(py::init<>())
            .def("generate_prime", &RSA::generate_prime, py::arg("hex_bit_count"),
                 "Generate a prime number with the given bit length")
            .def("encrypt", &RSA::encrypt, py::arg("message"),
                 "Encrypt a message using the public key")
            .def("decrypt", &RSA::decrypt, py::arg("cipher"),
                 "Decrypt a message using the private key")
            .def("sign", &RSA::sign, py::arg("digest"),
                 "Sign a digest using the private key")
            .def("verify", &RSA::verify, py::arg("digest"), py::arg("signature"),
                 "Verify a signature for a given digest")
            .def("generate_key_pair", &RSA::generate_key_pair, py::arg("len"),
                 "Generate an RSA key pair of the specified bit length");
}