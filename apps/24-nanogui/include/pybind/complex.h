/*
    pybind/complex.h: Complex number support

    Copyright (c) 2015 Wenzel Jakob <wenzel@inf.ethz.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#include <pybind/pybind.h>
#include <complex>

NAMESPACE_BEGIN(pybind)

PYBIND_DECL_FMT(std::complex<float>, "Zf");
PYBIND_DECL_FMT(std::complex<double>, "Zd");

NAMESPACE_BEGIN(detail)
template <typename T> class type_caster<std::complex<T>> {
public:
    bool load(PyObject *src, bool) {
        Py_complex result = PyComplex_AsCComplex(src);
        if (result.real == -1.0 && PyErr_Occurred()) {
            PyErr_Clear();
            return false;
        }
        value = std::complex<T>((T) result.real, (T) result.imag);
        return true;
    }

    static PyObject *cast(const std::complex<T> &src, return_value_policy /* policy */, PyObject * /* parent */) {
        return PyComplex_FromDoubles((double) src.real(), (double) src.imag());
    }

    PYBIND_TYPE_CASTER(std::complex<T>, "complex");
};
NAMESPACE_END(detail)
NAMESPACE_END(pybind)
