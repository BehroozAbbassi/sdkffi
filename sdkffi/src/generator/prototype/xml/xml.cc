#include "pch.h"
#include "xml.h"

namespace sdkffi::generator::prototype {
namespace xml {
bool
Xml::Generate(FormatSetting format_setting)
{
    PrintLogNormal("Start saving Xml...");

    {
        std::ofstream ofs(output_file_name_, std::ios::out);

#ifdef CEREAL_SERIALIZE
        cereal::XMLOutputArchive       archive(ofs);
        std::vector<types::Function> & functions = functions_;
        archive(CEREAL_NVP(functions));
#endif  // CEREAL_SERIALIZE
    }

    if (format_setting == FormatSetting::kEnableOutputFormat && formatter_)
        formatter_->Format();

    PrintLogNormal("Finish saving Xml");

    return true;
}
}  // namespace xml
}  // namespace sdkffi::generator::prototype