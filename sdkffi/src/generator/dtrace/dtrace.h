#pragma once

#include "types/data_types.h"
#include "generator/generator_base.h"

#include "generator/dtrace/command/command.h"

namespace sdkffi::generator {
namespace dtrace {

enum struct SyscallEvent
{
    kEntry,
    kReturn
};

enum struct Providers
{
    kDtrace,
    kSyscall,
    kProc,
    kProfile,
    kFbt,
    kLockstat
};

class Dtrace : IGenerator
{
public:
    Dtrace(const std::vector<types::Function> & parsed_functions, const std::string & file_name) :
        IGenerator(file_name, nullptr), functions_(parsed_functions) {}

    ModuleInfo GetInfo() const
    {
        return {
            "Dtrace",
            "v0.1 beta",
            "Try to generate DTrace script (Prob) loggers for Windows API's",
            "@BehroozAbbassi",

        };
    }

    std::string
    GetStructField(const types::StructField & field)
    {
        const auto kIndent = std::string(0 + 4, ' ');

        auto layout = fmt::format("/* {0:#04x} | {1:#04x} | {2:#04x} */",
                                  field.layout.offset,
                                  field.layout.size,
                                  field.layout.alignment);

        layout.clear();

        if (field.attributes.is_array)
        {
            return fmt::format("{0} {1} {2} {3} [{4}];\n", kIndent, layout, field.type, field.name, field.array_length);
        }
        else if (field.attributes.is_bit_field)
        {
            return fmt::format("{0} {1} {2} {3} : {4};\n", kIndent, layout, field.type, field.name, field.bit_field);
        }
        else
        {
            return fmt::format("{0} {1} {2} {3};\n", kIndent, layout, field.type, field.name);
        }
    }

    std::string
    GetUnionField(const types::UnionField & field)
    {
        const auto kIndent = std::string(0 + 4, ' ');

        auto layout = fmt::format("/* {0:#04x} | {1:#04x} | {2:#04x} */",
                                  field.layout.offset,
                                  field.layout.size,
                                  field.layout.alignment);

        layout.clear();

        if (field.attributes.is_array)
        {
            return fmt::format("{0} {1} {2} {3} [{4}];\n", kIndent, layout, field.type, field.name, field.array_length);
        }
        else if (field.attributes.is_bit_field)
        {
            return fmt::format("{0} {1} {2} {3} : {4};\n", kIndent, layout, field.type, field.name, field.bit_field);
        }
        else
        {
            return fmt::format("{0} {1} {2} {3};\n", kIndent, layout, field.type, field.name);
        }
    }

    void GetStructDecl(const types::Struct & input, std::ofstream & ofs)
    {
        ofs << fmt::format("struct {0} {{\n", input.name);

        for (auto field : input.dyn_fields)
        {
            if (field.type == types::FieldType::kStructField)
            {
                types::StructField & t = std::any_cast<types::StructField &>(field.value);
                ofs << GetStructField(t);
            }
            else if (field.type == types::FieldType::kStruct)
            {
                types::Struct & t = std::any_cast<types::Struct &>(field.value);
                GetStructDecl(t, ofs);
            }
            else if (field.type == types::FieldType::kUnion)
            {
                auto t = std::any_cast<types::Union &>(field.value);
                GetUnionDecl(t, ofs);
            }
        }
        ofs << fmt::format("}}{0}; /* struct {1} */\n",
                           input.instance_name,
                           // input.typedef_name.alias,
                           input.name);

        // if (input.attributes.has_alias)
        //{
        //     ofs << fmt::format("typedef {0} {1};\n",
        //                        input.typedef_name.alias,
        //                        input.typedef_name.type_name);
        // }

        ofs << "\n\n";
    }

    void
    GetUnionDecl(const types::Union & union_def, std::ofstream & ofs)
    {
        ofs << fmt::format("union {0} {{\n", union_def.name);

        for (auto field : union_def.dyn_fields)
        {
            if (field.type == types::FieldType::kUnionField)
            {
                // auto t =
                // models::type_kind::GetField<models::type_kind::TypeKind::kUnionField>(field);

                types::UnionField & t = std::any_cast<types::UnionField &>(field.value);
                ofs << GetUnionField(t);
            }
            else if (field.type == types::FieldType::kStruct)
            {
                types::Struct & t = std::any_cast<types::Struct &>(field.value);
                GetStructDecl(t, ofs);
            }
            else if (field.type == types::FieldType::kUnion)
            {
                auto t = std::any_cast<types::Union &>(field.value);
                GetUnionDecl(t, ofs);
            }
        }
        ofs << fmt::format("}}{0}; /* union {1} */\n",
                           union_def.instance_name,
                           union_def.name);
    }

    std::vector<std::string> GetCommands()
    {
        std::vector<std::string> result = {};
        for (const auto & fn : functions_)
        {
            const auto command     = GetDtraceCommand(fn);
            const auto command_str = command.ToString();
            // std::cout << command_str  << "\n";

            result.emplace_back(command_str);
        }

        return result;
    }

    void SetTypes(types::TypesGraph types)
    {
        types_ = types;
    }

    std::vector<std::string> GetTypedefs()
    {
        std::vector<std::string> result;

        for (const auto & t : types_.typedefs)
        {
            result.emplace_back(fmt::format("typedef {} {};", t.type_name, t.alias));
        }

        return result;
    }

    template <typename _Ty>
    std::optional<_Ty> GetType(const types::GenericField & dyn,
                               const types::FieldType &    kind)
    {
        if (dyn.type == kind)
        {
            return std::any_cast<_Ty>(dyn.value);
        }

        return std::nullopt;
    }

    std::string GetHeaderComment();

    bool Generate(FormatSetting formatSetting) override;

private:
    std::vector<types::Function> functions_;
    types::TypesGraph            types_;

    const char * kLocalParameterNamePrefix = "Arg_";
    const char * kIndent                   = "    ";

    std::string GetFormattedFunctionDeclaration(const types::Function & function) const;

    std::string GetParameters(const types::Function & function);

    std::string GenerateLocalParameters(const types::Function & function);

    std::string GenerateLoggerFunction(const types::Function & function);

    std::string GenerateActionCode(const types::Function & function);

    Command GetDtraceCommand(const types::Function & function);
};

}  // namespace dtrace

}  // namespace sdkffi::generator