#pragma once

namespace utils::string {

inline std::string
CxStringToString(const CXString & s)
{
    std::string result = clang_getCString(s);
    clang_disposeString(s);
    return result;
}
}  // namespace utils::string
