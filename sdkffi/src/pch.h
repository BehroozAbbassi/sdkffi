#pragma once

// -----------------------------------------------------------------------------
// Windows API headers
//

#define NOMINMAX
//#define CINTERFACE

#pragma region Win32 API Headers

#include <windows.h>
#include <tchar.h>

#pragma endregion = > Win32 API Headers

// -----------------------------------------------------------------------------
// stl headers
//

#pragma region C++ Standrad Headers

#include <algorithm>
#include <execution>
#include <any>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

#ifdef _HAS_CXX17
#    include <filesystem>
#    include <optional>
namespace fs = std::filesystem;
#endif  // _HAS_CXX17

#ifdef _HAS_CXX14
#    include <filesystem>
namespace fs = std::experimental::filesystem;
#endif  // _HAS_CXX14

#pragma endregion = > C++ Standrad Headers

// -----------------------------------------------------------------------------
// third-party library headers
//

//
// vcpkg.exe install cereal:x64-windows cereal:x86-windows
// vcpkg.exe install nlohmann-json:x64-windows nlohmann-json:x86-windows
// vcpkg.exe install rang:x64-windows rang:x86-windows
// vcpkg.exe install cxxopts:x64-windows cxxopts:x86-windows
// vcpkg.exe install ms-gsl:x64-windows ms-gsl:x86-windows
// vcpkg.exe install fmt:x64-windows fmt:x86-windows
//

//
// LLVM/Clang header files
//

#pragma region LLVM / Clang Headers

#include <clang-c/Index.h>
#include <clang-c/Platform.h>

#pragma comment(lib, "libclang.lib")
#pragma comment(lib, "libomp.lib")

#pragma endregion

//
// Serialization
//

#define ENABLE_LIB_CEREAL

#ifdef ENABLE_LIB_CEREAL
#    ifndef CINTERFACE
#        define CINTERFACE
#    endif

#    pragma warning(push)
#    pragma warning(disable : 26495)
#    pragma warning(disable : 26819)  // Warning C26819: Unannotated fallthrough between switch labels
#    pragma warning(disable : 26812)  // Warning C26812: Enum type is unscoped. Prefer enum class over enum

#    include <cereal/archives/json.hpp>
#    include <cereal/archives/xml.hpp>
#    include <cereal/types/memory.hpp>
#    include <cereal/types/set.hpp>
#    include <cereal/types/unordered_map.hpp>
#    include <cereal/types/vector.hpp>

#    pragma warning(pop)

#    ifdef CINTERFACE
#        undef CINTERFACE
#    endif

#    define CEREAL_SERIALIZE
#endif

#include <nlohmann/json.hpp>

//
// Command line parser
//

#include <fmt/format.h>  // string format

#include <rang.hpp>  // color console

#include <cxxopts.hpp>
#include <gsl/gsl>

// -----------------------------------------------------------------------------
// Project internal headers
//

#include "util/logger.h"
#include "util/string.h"

namespace utils::libclang {

inline bool
IsForwardDeclaration(CXCursor cursor)
{
    auto definition = clang_getCursorDefinition(cursor);
    //
    // If the definition is null, then there is no definition in this translation
    // unit, so this cursor must be a forward declaration.
    //
    if (clang_equalCursors(definition, clang_getNullCursor()))
        return true;

    //
    // If there is a definition, then the forward declaration and the definition
    // are in the same translation unit. This cursor is the forward declaration if
    // it is _not_ the definition.
    //
    return !clang_equalCursors(cursor, definition);
}

inline bool
IsFromFileToInspect(CXCursor cursor)
{
    return static_cast<bool>(clang_Location_isFromMainFile(clang_getCursorLocation(cursor)));
}

inline bool
IsAnonymousType(CXCursor cursor)
{
    if (clang_Cursor_isAnonymous(cursor))
        return true;

    CXType      cursor_type = clang_getCursorType(cursor);
    std::string data_tye    = utils::string::CxStringToString(clang_getTypeSpelling(cursor_type));

    // For a reason I don't understand, clang_Cursor_isAnonymous returns false
    // when an anonymous type_kind has an int32_t field. Add another check for the alias
    // of the type_kind to ensure that we don't process the anonymous type_kind.
    if (data_tye.find("(anonymous") != std::string::npos)
        return true;

    return false;
}

}  // namespace utils::libclang

//#include "models.h"