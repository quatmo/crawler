/*
    pybind/common.h -- Basic macros

    Copyright (c) 2015 Wenzel Jakob <wenzel@inf.ethz.ch>

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE file.
*/

#pragma once

#if !defined(NAMESPACE_BEGIN)
#define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END)
#define NAMESPACE_END(name) }
#endif

#if !defined(PYTHON_EXPORT)
#if defined(_WIN32)
#define PYTHON_EXPORT __declspec(dllexport)
#else
#define PYTHON_EXPORT __attribute__ ((visibility("default")))
#endif
#endif

#include <vector>
#include <string>
#include <stdexcept>
#include <unordered_map>
#include <memory>

/// Include Python header, disable linking to pythonX_d.lib on Windows in debug mode
#if defined(_MSC_VER)
#define HAVE_ROUND
#pragma warning(push)
#pragma warning(disable: 4510 4610 4512 4005)
#if _DEBUG
#define _DEBUG_MARKER
#undef _DEBUG
#endif
#endif
#include <Python.h>
#ifdef isalnum
#undef isalnum
#undef isalpha
#undef islower
#undef isspace
#undef isupper
#undef tolower
#undef toupper
#endif
#if defined(_MSC_VER)
#if defined(_DEBUG_MARKER)
#define _DEBUG
#undef _DEBUG_MARKER
#endif
#pragma warning(pop)
#endif

#if PY_MAJOR_VERSION >= 3
#define PYTHON_PLUGIN(name) \
    extern "C" PYTHON_EXPORT PyObject *PyInit_##name()
#else
#define PYTHON_PLUGIN(name) \
    extern "C" PYTHON_EXPORT PyObject *init##name()
#endif

NAMESPACE_BEGIN(pybind)

typedef Py_ssize_t ssize_t;

/// Approach used to cast a previously unknown C++ instance into a Python object
enum class return_value_policy : int {
    /** Automatic: copy objects returned as values and take ownership of objects
        returned as pointers */
    automatic = 0,
    /** Reference the object and take ownership. Python will call the
        destructor and delete operator when the reference count reaches zero */
    take_ownership,
    /** Reference the object, but do not take ownership (dangerous when C++ code
        deletes it and Python still has a nonzero reference count) */
    reference,
    /** Reference the object, but do not take ownership. The object is considered
        be owned by the C++ instance whose method or property returned it. The
        Python object will increase the reference count of this 'parent' by 1 */
    reference_internal,
    /// Create a new copy of the returned object, which will be owned by Python
    copy
};

/// Format strings for basic number types
template <typename type> struct format_descriptor { };
#define PYBIND_DECL_FMT(t, n) template<> struct format_descriptor<t> { static std::string value() { return n; }; };
PYBIND_DECL_FMT(int8_t,  "b"); PYBIND_DECL_FMT(uint8_t,  "B"); PYBIND_DECL_FMT(int16_t, "h"); PYBIND_DECL_FMT(uint16_t, "H");
PYBIND_DECL_FMT(int32_t, "i"); PYBIND_DECL_FMT(uint32_t, "I"); PYBIND_DECL_FMT(int64_t, "q"); PYBIND_DECL_FMT(uint64_t, "Q");
PYBIND_DECL_FMT(float,   "f"); PYBIND_DECL_FMT(double,   "d"); PYBIND_DECL_FMT(bool,    "?");

/// Information record describing a Python buffer object
struct buffer_info {
    void *ptr;
    size_t itemsize, count;
    std::string format; // for dense contents, this should be set to format_descriptor<T>::value
    int ndim;
    std::vector<size_t> shape;
    std::vector<size_t> strides;

    buffer_info(void *ptr, size_t itemsize, const std::string &format, int ndim,
                const std::vector<size_t> &shape, const std::vector<size_t> &strides)
        : ptr(ptr), itemsize(itemsize), format(format), ndim(ndim),
          shape(shape), strides(strides) {
        count = 1; for (int i=0; i<ndim; ++i) count *= shape[i];
    }
};

// C++ bindings of core Python exceptions
struct stop_iteration    : public std::runtime_error { public: stop_iteration(const std::string &w="") : std::runtime_error(w)   {} };
struct index_error       : public std::runtime_error { public: index_error(const std::string &w="")    : std::runtime_error(w)   {} };
struct error_already_set : public std::exception     { public: error_already_set()                                               {} };
/// Thrown when pybind::cast or handle::call fail due to a type casting error
struct cast_error        : public std::runtime_error { public: cast_error(const std::string &w = "") : std::runtime_error(w) {} };

NAMESPACE_BEGIN(detail)

/// PyObject wrapper around generic types
template <typename type, typename holder_type = std::unique_ptr<type>> struct instance {
    PyObject_HEAD
    type *value;
    PyObject *parent;
    bool owned : 1;
    bool constructed : 1;
    holder_type holder;
};

/// Additional type information which does not fit into the PyTypeObjet
struct type_info {
    PyTypeObject *type;
    size_t type_size;
    void (*init_holder)(PyObject *);
    std::vector<PyObject *(*)(PyObject *, PyTypeObject *)> implicit_conversions;
    buffer_info *(*get_buffer)(PyObject *, void *) = nullptr;
    void *get_buffer_data = nullptr;
};

/// Internal data struture used to track registered instances and types
struct internals {
    std::unordered_map<const std::type_info *, type_info> registered_types;
    std::unordered_map<void *, PyObject *> registered_instances;
};

/// Return a reference to the current 'internals' information
inline internals &get_internals();

/// Index sequence for convenient template metaprogramming involving tuples
template<size_t ...> struct index_sequence  { };
template<size_t N, size_t ...S> struct make_index_sequence : make_index_sequence <N - 1, N - 1, S...> { };
template<size_t ...S> struct make_index_sequence <0, S...> { typedef index_sequence<S...> type; };

/// Strip the class from a method type
template <typename T> struct remove_class {};
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...)> { typedef R type(A...); };
template <typename C, typename R, typename... A> struct remove_class<R (C::*)(A...) const> { typedef R type(A...); };

/// Helper template to strip away type modifiers
template <typename T> struct decay                       { typedef T type; };
template <typename T> struct decay<const T>              { typedef typename decay<T>::type type; };
template <typename T> struct decay<T*>                   { typedef typename decay<T>::type type; };
template <typename T> struct decay<T&>                   { typedef typename decay<T>::type type; };
template <typename T> struct decay<T&&>                  { typedef typename decay<T>::type type; };
template <typename T, size_t N> struct decay<const T[N]> { typedef typename decay<T>::type type; };
template <typename T, size_t N> struct decay<T[N]>       { typedef typename decay<T>::type type; };

/// Helper type to replace 'void' in some expressions
struct void_type { };

NAMESPACE_END(detail)
NAMESPACE_END(pybind)
