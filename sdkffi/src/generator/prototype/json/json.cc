#include "pch.h"
#include "json.h"

namespace sdkffi::generator::prototype {
namespace json {
bool
Json::Generate(FormatSetting format_setting)
{
    PrintLogNormal("Start saving Json...");

    {
        std::ofstream ofs(output_file_name_, std::ios::out);

#ifdef CEREAL_SERIALIZE
        cereal::JSONOutputArchive      archive(ofs);
        std::vector<types::Function> & functions = functions_;
        archive(CEREAL_NVP(functions));
#endif  // CEREAL_SERIALIZE
    }

    if (format_setting == FormatSetting::kEnableOutputFormat && formatter_)
        formatter_->Format();

    PrintLogNormal("Finish saving Json");

    return true;
}
}  // namespace json
}  // namespace sdkffi::generator::prototype