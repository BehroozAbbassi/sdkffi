#pragma once

#include "types/data_types.h"

namespace sdkffi::generator::pretty {

inline std::string
GetWhiteSpaceDelimeter(const int max_length, const int curr_length, const int extra_space)
{
    return std::string((max_length - curr_length) + extra_space, ' ');
}

class Function
{
public:
    Function(const types::Function & function) :
        function_(function)
    {
    }

    size_t GetMaxParameterSalLength() const
    {
        size_t max_name_delim_length = 0;
        for (const auto & param : function_.arguments)
        {
            if (max_name_delim_length < param.annotation.length())
                max_name_delim_length = param.annotation.length();
        }

        return max_name_delim_length;
    }

    size_t GetMaxParameterNameLength() const
    {
        size_t max_name_delim_length = 0;
        for (const auto & param : function_.arguments)
        {
            if (max_name_delim_length < param.name.length())
                max_name_delim_length = param.name.length();
        }

        return max_name_delim_length;
    }

    size_t GetMaxParameterTypeLength() const
    {
        size_t max_type_delim_length = 0;
        for (const auto & param : function_.arguments)
        {
            if (max_type_delim_length < param.type.length())
                max_type_delim_length = param.type.length();
        }

        return max_type_delim_length;
    }

private:
    types::Function function_;
};
}  // namespace sdkffi::generator::pretty