#pragma once

#include "crc32.hpp"

#include <string>

class HashedString
{
public:
    explicit HashedString(const std::string& string);
    ~HashedString() =default;

    bool operator == (const HashedString& rhs) const { return mHash == rhs.mHash && mString == rhs.mString; }
    bool operator != (const HashedString& rhs) const { return mHash != rhs.mHash || mString != rhs.mString; }

#define HashedString_comparator(op) bool operator op (const HashedString& rhs) const { return mHash op rhs.mHash; }
    HashedString_comparator(<);
    HashedString_comparator(<=);
    HashedString_comparator(>);
    HashedString_comparator(>=);
#undef HashedString_comparator

private:
    std::string mString;
    crc32_t mHash;
};
