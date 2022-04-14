#include "pch.h"

#include "app/options.h"
#include "types/data_types.h"
#include "clang/configs.h"
#include "clang/libclang.h"

#include "parser/parser.h"
#include "generator/generator.h"

namespace sdkffi::app {
bool
main(int argc, char * argv[])
{
    std::optional<Options> options = ParseCommandline(argc, argv);
    if (options.has_value() == false)
        return false;

    Options cmd_options = options.value();

    const auto output_proj_dir  = fs::path(cmd_options.output_directory_path).append(cmd_options.project_name);
    auto       output_file_name = (output_proj_dir / fs::path(cmd_options.input_file_name).filename());

    fs::create_directories(output_proj_dir);

    clang::configs::LibclangConfigs clang_configs;
    if (clang_configs.ReadFromFile("clang_configs.json") == false)
        return false;

    clang_configs.file_name = cmd_options.input_file_name;

    clang::Libclang libclang(clang_configs);
    if (libclang.Initialize() == false)
        return false;

    //
    // Notice: in case we want to have a deep view of data types and their
    //  dependencies, we need to parse data types (typedef, struct, union, enum) recursively
    //  in all header files and not the file that provided as input file.
    //
    parser::type::Options type_parser_options;
    type_parser_options.recusrive_mode = parser::type::RecursiveMode::kDisable;
    type_parser_options.recusrive_mode = parser::type::RecursiveMode::kEnable;

    parser::function::Options func_parser_options;
    func_parser_options.recusrive_mode = parser::function::RecursiveMode::kEnable;
    func_parser_options.recusrive_mode = parser::function::RecursiveMode::kDisable;

    types::TypesGraph            parsed_types     = {};
    std::vector<types::Function> parsed_functions = {};

    auto libclang_state = libclang.GetState();

    parser::type::TypeParser types_parser(libclang_state, type_parser_options);
    if (types_parser.StartParsing())
    {
        parsed_types.typedefs           = types_parser.GetTypedefs();
        parsed_types.structs            = types_parser.GetStructs();
        parsed_types.unions             = types_parser.GetUnions();
        parsed_types.enums              = types_parser.GetEnums();
        parsed_types.dummy_enum_names   = types_parser.GetEnumDummyNames();
        parsed_types.dummy_struct_names = types_parser.GetStructDummyNames();
        parsed_types.dummy_union_names  = types_parser.GetUnionDummyNames();

        std::cout << "number of parsed typedefs is : " << parsed_types.typedefs.size() << "\n";
        std::cout << "number of parsed structs is  : " << parsed_types.structs.size() << "\n";
        std::cout << "number of parsed unions is   : " << parsed_types.unions.size() << "\n";
        std::cout << "number of parsed enums is    : " << parsed_types.enums.size() << "\n";

        // parser::dd::MapTypedefsToStruct(parsed_types.typedefs, parsed_types.structs);

        const auto                dtrace_file_name = output_file_name.replace_extension(".c").string();
        generator::dtrace::Dtrace dtrace_generator(parsed_functions, dtrace_file_name);

        dtrace_generator.SetTypes(parsed_types);
        dtrace_generator.Generate(generator::FormatSetting::kDisableOutputFormat);
    }

    parser::function::Parser func_parser(libclang_state, func_parser_options);
    if (func_parser.StartParsing())
    {
        parsed_functions = func_parser.GetFunctionsByRef();
        std::cout << "number of parsed functions is : " << parsed_functions.size() << "\n";

        const auto                dtrace_file_name = output_file_name.replace_extension(".d").string();
        generator::dtrace::Dtrace dtrace_generator(parsed_functions, dtrace_file_name);

        dtrace_generator.SetTypes(parsed_types);
        dtrace_generator.Generate(generator::FormatSetting::kDisableOutputFormat);

        const auto json_file_name = output_file_name.replace_extension(".json").string();
        {
            std::ofstream             ofs(json_file_name, std::ios::out);
            cereal::JSONOutputArchive archive(ofs);

            archive(CEREAL_NVP(parsed_functions));
        }
    }

    const auto                       json_file_name = output_file_name.replace_extension(".json").string();
    generator::prototype::json::Json json_generator(parsed_functions, json_file_name);
    json_generator.Generate(generator::FormatSetting::kDisableOutputFormat);

    const auto                     xml_file_name = output_file_name.replace_extension(".xml").string();
    generator::prototype::xml::Xml xml_generator(parsed_functions, xml_file_name);
    xml_generator.Generate(generator::FormatSetting::kDisableOutputFormat);

    return true;
}

}  // namespace sdkffi::app

int
main(int argc, char * argv[])
{
    return sdkffi::app::main(argc, argv);
}