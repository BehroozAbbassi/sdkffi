
#include "pch.h"
#include "configs.h"
#include "libclang.h"

namespace sdkffi::clang {
bool
Libclang::Initialize()
{
    auto &               clangArgs  = libclang_configs_.clang_args;
    const char * const * clang_args = clangArgs.GetArgs();

    index_ = clang_createIndex(0, 0);

    // TODO : deal with preprocessor condition directives
    // CXTranslationUnit_DetailedPreprocessingRecord
    // CXTranslationUnit_RetainExcludedConditionalBlocks
    //
    //
    // -detailed-preprocessing-record
    //
    // default_parser_options = (
    //     CXTranslationUnit_DetailedPreprocessingRecord | # needed for
    //     preprocessing parsing CXTranslationUnit_SkipFunctionBodies | # for
    //     faster parsing CXTranslationUnit_SingleFileParse | # don't parse
    //     include files recursively
    //     CXTranslationUnit_RetainExcludedConditionalBlocks | # keep includes
    //     inside ifdef blocks CXTranslationUnit_KeepGoing  # don't stop on errors
    //     )

    uint32_t translation_flags = (
        // needed for preprocessing parsing
        CXTranslationUnit_Flags::CXTranslationUnit_DetailedPreprocessingRecord |
        CXTranslationUnit_Flags::
            CXTranslationUnit_SkipFunctionBodies |  // for faster parsing
        // CXTranslationUnit_Flags::CXTranslationUnit_SingleFileParse | // don't
        // parse include files recursively
        CXTranslationUnit_Flags::
            CXTranslationUnit_RetainExcludedConditionalBlocks |  // keep includes
                                                                 // inside ifdef
                                                                 // blocks
        CXTranslationUnit_Flags::CXTranslationUnit_KeepGoing     // don't stop on
                                                                 // errors
    );

    // translation_flags = CXTranslationUnit_Flags::CXTranslationUnit_None;
    translation_unit_ = clang_parseTranslationUnit(
        index_,
        libclang_configs_.file_name.c_str(),
        clang_args,
        clangArgs.GetArgsCount(),
        nullptr,
        0,
        translation_flags);

    if (translation_unit_ == nullptr)
    {
        PrintLogError("Unable to parse translation unit. Quitting.");
        return false;
    }

    PrintLibclangDiagnostics(translation_unit_);

    root_cursor_ = clang_getTranslationUnitCursor(translation_unit_);

    is_initialized_ = true;

    return is_initialized_;
}
bool
Libclang::Finitialize()
{
    clang_disposeTranslationUnit(translation_unit_);
    clang_disposeIndex(index_);

    return true;
}
}  // namespace sdkffi::clang