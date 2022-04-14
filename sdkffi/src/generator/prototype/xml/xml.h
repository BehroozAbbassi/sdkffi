#pragma once

#include "types/data_types.h"
#include "generator/generator_base.h"

namespace sdkffi::generator::prototype {
namespace xml {

class Xml : IGenerator
{
public:
    Xml(const std::vector<types::Function> & functions,
        const std::string &                  output_file_name) :
        IGenerator(output_file_name, nullptr), functions_(functions)
    {
    }

    bool Generate(FormatSetting format_setting) override;

    ModuleInfo GetInfo() const
    {
        return {
            "Xml",
            "v0.1 beta",
            "Xml serialization of parsed data",
            "@BehroozAbbassi",

        };
    }

private:
    std::vector<types::Function> functions_;
};
}  // namespace xml

}  // namespace sdkffi::generator::prototype
