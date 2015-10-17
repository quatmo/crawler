/*
    pybind/functional.h: std::function<> support

    Copyright (c) 2015 Wenzel Jakob <wenzel@inf.ethz.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#include <pybind/pybind.h>
#include <functional>

NAMESPACE_BEGIN(pybind)
NAMESPACE_BEGIN(detail)

template <typename Return, typename... Args> struct type_caster<std::function<Return(Args...)>> {
    typedef std::function<Return(Args...)> type;
public:

    bool load(PyObject *src_, bool) {
        if (!PyFunction_Check(src_))
            return false;
        object src(src_, true);
        value = [src](Args... args) -> Return {
            object retval(pybind::handle(src).call(std::move(args)...));
            if (retval.ptr() == nullptr && PyErr_Occurred())
                throw error_already_set();
            /* Visual studio 2015 parser issue: need parentheses around this expression */
            return (retval.template cast<Return>());
        };
        return true;
    }

    template <typename Func>
    static PyObject *cast(Func &&f_, return_value_policy policy, PyObject *) {
        cpp_function f(std::forward<Func>(f_), policy);
        f.inc_ref();
        return f.ptr();
    }


    PYBIND_TYPE_CASTER(type, detail::descr("function<") +
            type_caster<std::tuple<Args...>>::name() + detail::descr(" -> ") +
            type_caster<typename decay<Return>::type>::name() +
            detail::descr(">"));
};

NAMESPACE_END(detail)
NAMESPACE_END(pybind)
