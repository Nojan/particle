#pragma once

#include "crc32.hpp"

#include <string>

#define HashedString_comparator(op) bool operator op (const HashedString& rhs) const { return mHash op rhs.mHash && mString op rhs.mString; }

class HashedString
{
public:
    explicit HashedString(const std::string& string);
    ~HashedString() =default;

    HashedString_comparator(==);
    HashedString_comparator(!=);
    HashedString_comparator(<);
    HashedString_comparator(<=);
    HashedString_comparator(>);
    HashedString_comparator(>=);

private:
    std::string mString;
    crc32_t mHash;
};

#undef HashedString_comparator