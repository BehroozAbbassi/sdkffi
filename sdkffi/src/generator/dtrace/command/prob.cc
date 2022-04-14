#include "pch.h"

#include "prob.h"

namespace sdkffi {

namespace generator {
namespace dtrace {

std::string
Probe::ToString() const
{
    std::stringstream ss;

    if (provider.empty() == false)
    {
        ss << provider << ":";

        if (module.empty() == false)
            ss << module << ":";
        else
            ss << ":";

        if (function.empty() == false)
            ss << function << ":";
        else
            ss << ":";

        if (name.empty() == false)
            ss << name;
    }

    return ss.str();
}
}  // namespace dtrace
}  // namespace generator
}  // namespace sdkffi