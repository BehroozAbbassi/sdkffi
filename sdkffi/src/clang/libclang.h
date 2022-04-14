#pragma once
#include "clang/configs.h"
namespace sdkffi::clang {

struct LibclangState
{
    CXCursor          root_cursor;
    CXTranslationUnit translation_unit;
    CXIndex           index;
    bool              initialized = false;
};

class Libclang
{
public:
    Libclang(configs::LibclangConfigs & configs) :
        libclang_configs_(configs) {}

    bool Initialize();
    bool Finitialize();

    LibclangState GetState() const
    {
        LibclangState result;

        result.root_cursor      = root_cursor_;
        result.translation_unit = translation_unit_;
        result.index            = index_;
        result.initialized      = is_initialized_;

        return result;
    }

private:
    CXCursor                 root_cursor_;
    CXTranslationUnit        translation_unit_;
    CXIndex                  index_;
    bool                     is_initialized_ = false;
    configs::LibclangConfigs libclang_configs_;
};
}  // namespace sdkffi::clang