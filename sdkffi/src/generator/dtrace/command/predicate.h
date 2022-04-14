#pragma once

namespace sdkffi::generator {
namespace dtrace {
class Predicate
{
public:
    Predicate() = default;

    Predicate(const std::string & predicate)
    {
        if (predicate.empty())
            return;

        value = {};
        value.append(kPredicateBeginToken);
        value.append("\n");
        value.append(predicate);
        value.append("\n");
        value.append(kPredicateEndToken);
    }

    std::string value;

private:
    const char * kPredicateBeginToken = "/";
    const char * kPredicateEndToken   = "/";
};
}  // namespace dtrace
}  // namespace sdkffi::generator