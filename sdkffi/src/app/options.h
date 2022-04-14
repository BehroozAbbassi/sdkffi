
#include "clang/configs.h"

namespace sdkffi::app {
struct Options
{
    std::string                        input_file_name{};
    std::string                        output_directory_path{};
    std::string                        project_name{};
    clang::configs::RecusriveVisitMode recursive_mode;
};

std::optional<Options>
ParseCommandline(int argc, char * argv[]);

}  // namespace sdkffi::app