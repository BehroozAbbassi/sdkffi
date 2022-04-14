#pragma once

namespace sdkffi {

namespace generator {
namespace dtrace {
class Comment
{
public:
    Comment(){};
    Comment(const std::string & comment)
    {
        if (comment.empty())
            return;

        value = {};
        value.append(kCommentBeginToken);
        value.append("\n");
        value.append(comment);
        value.append("\n");
        value.append(kCommentEndToken);
    }

    // std::string GetValue() const { return value; }

    // explicit operator std::string() const { return value; }
    // explicit operator const char *() const { return value.c_str(); }

    std::string value;

private:
    const char * kCommentBeginToken = "/*";
    const char * kCommentEndToken   = "*/";
};
}  // namespace dtrace
}  // namespace generator
}  // namespace sdkffi