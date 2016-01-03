#include "HashedString.hpp"

HashedString::HashedString(const std::string& string)
    : mString(string)
    , mHash(crc::hash(string.c_str()))
{
}

