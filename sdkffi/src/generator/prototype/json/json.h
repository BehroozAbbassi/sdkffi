#pragma once

#include "types/data_types.h"
#include "generator/generator_base.h"

namespace sdkffi::generator::prototype {
namespace json {

class Json : IGenerator
{
public:
    Json(const std::vector<types::Function> & functions,
         const std::string &                  output_file_name) :
        IGenerator(output_file_name, nullptr), functions_(functions)
    {
    }

    bool Generate(FormatSetting format_setting) override;

    ModuleInfo GetInfo() const
    {
        return {
            "Json",
            "v0.1 beta",
            "Json serialization of parsed data",
            "@BehroozAbbassi",

        };
    }

private:
    std::vector<types::Function> functions_;
};
}  // namespace json

}  // namespace sdkffi::generator::prototype
