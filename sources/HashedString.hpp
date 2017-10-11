#pragma once

#include "crc32.hpp"

#include <string>

class HashedString
{
public:
    using hash_type = crc32_t;

    explicit HashedString(const std::string& string);
    explicit HashedString(const char* string);
    ~HashedString() =default;

    const std::string& string() const { return mString; }
    hash_type hash() const { return mHash; }

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
    hash_type mHash;
};

namespace std
{
    template<> struct hash<HashedString>
    {
        typedef HashedString argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& s) const noexcept
        {
            return s.hash();
        }
    };
}