#ifndef _FEATURES_HPP_INCLUDED_
#define _FEATURES_HPP_INCLUDED_

// https://en.cppreference.com/w/cpp/feature_test
#if defined(__has_cpp_attribute)
#    define HAS_CPP_ATTR(attr) __has_cpp_attribute(attr)
#else
#    define HAS_CPP_ATTR (0)
#endif

#define NODISCARD_CPP17 201603L
#define NODISCARD_CPP19 201907L
#if HAS_CPP_ATTR(nodiscard) == NODISCARD_CPP17
#    define NODISCARD(msg) [[nodiscard]]
#elif HAS_CPP_ATTR(nodiscard) == NODISCARD_CPP19
#    define NODISCARD(msg) [[nodiscard(msg)]]
#else
#    define NODISCARD(msg)
#endif

#endif // _FEATURES_HPP_INCLUDED_