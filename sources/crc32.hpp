#pragma once
#include "phantom_pod.hpp"
#include "types.hpp"

// source: http://stackoverflow.com/questions/28675727/using-crc32-algorithm-to-hash-string-at-compile-time

DECLARE_PHANTOM_TYPE(uint32_t, crc32_t, phantom::traits::comparable | phantom::traits::orderable);

namespace crc {

// Generate CRC lookup table
template <unsigned c, int k = 8>
struct f : f<((c & 1) ? 0xedb88320 : 0) ^ (c >> 1), k - 1> {};
template <unsigned c> struct f<c, 0>{enum {value = c};};

#define A(x) B(x) B(x + 128)
#define B(x) C(x) C(x +  64)
#define C(x) D(x) D(x +  32)
#define D(x) E(x) E(x +  16)
#define E(x) F(x) F(x +   8)
#define F(x) G(x) G(x +   4)
#define G(x) H(x) H(x +   2)
#define H(x) I(x) I(x +   1)
#define I(x) f<x>::value ,

constexpr unsigned table[] = { A(0) };

#undef I
#undef H
#undef G
#undef F
#undef E
#undef D
#undef C
#undef B
#undef A

// Constexpr implementation and helpers
constexpr uint32_t impl(const uint8_t* p, size_t len, uint32_t crc) {
    return len ?
        impl(p+1,len-1,(crc>>8)^table[(crc&0xFF)^*p])
        : crc;
}

constexpr uint32_t crc32(const uint8_t* data, size_t length) {
    return ~impl(data, length, ~0);
}

constexpr size_t strlen_c(const char* str) {
    return *str ? 1+strlen_c(str+1) : 0;
}

constexpr crc32_t hash(const char* str) {
    return crc32_t(crc32((uint8_t*)str, strlen_c(str)));
}

}