#include "pch.h"
#include "options.h"

namespace sdkffi::app {

std::optional<Options>
ParseCommandline(int argc, char * argv[])
{
    Options result;

    cxxopts::Options options("SDK FFI", "(^_^)");
    options.add_options()("f,file", "Input file path", cxxopts::value<std::string>());
    options.add_options()("r,recursive", "recursive mode", cxxopts::value<bool>());
    options.add_options()("o,output", "Output directory path", cxxopts::value<std::string>());
    options.add_options()("p,project", "project name", cxxopts::value<std::string>());

    auto cmd_result = options.parse(argc, argv);
    try
    {
        result.input_file_name       = cmd_result["file"].as<std::string>();
        result.output_directory_path = cmd_result["output"].as<std::string>();
        result.project_name          = cmd_result["project"].as<std::string>();
/*            result.recursive_mode =
                static_cast<libclang::Configs::RecusriveVisitMode>(cmd_result["recursive"].as<bool>());
       */ }
catch (...)
{
    std::cout << options.help();
    return std::nullopt;
}

return result;
}
}  // namespace sdkffi::app