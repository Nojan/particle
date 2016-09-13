#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <type_traits>

typedef unsigned int uint;

template<typename To, typename From>
To numeric_cast(From value)
{
    static_assert( std::is_arithmetic<From>::value, "");
    static_assert( std::is_arithmetic<To>::value, "");
    const To ToValue = static_cast<To>(value);
    assert(static_cast<From>(ToValue) == value);
    return ToValue;
}

#endif
