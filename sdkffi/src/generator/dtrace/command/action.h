#pragma once

namespace sdkffi {

namespace generator {
namespace dtrace {

class Action
{
public:
    Action() = default;

    explicit Action(const std::string & action)
    {
        if (action.empty())
            return;

        value = {};
        value.append(kActionBeginToken);
        value.append("\n");
        value.append(action);
        value.append("\n");
        value.append(kActionEndToken);
    }

    Action(const std::string action)
    {
        if (action.empty())
            return;

        value = {};
        value.append(kActionBeginToken);
        value.append("\n");
        value.append(action);
        value.append("\n");
        value.append(kActionEndToken);
    }

    std::string value;

private:
    const char * kActionBeginToken = "{";
    const char * kActionEndToken   = "}";
};
}  // namespace dtrace
}  // namespace generator
}  // namespace sdkffi