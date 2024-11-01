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
    py::class_<RSA<BigInt>>(variable, "RSA")
        .def(py::init<>())
        .def("generate_prime", &RSA<BigInt>::generate_prime);
}