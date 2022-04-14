#pragma once

namespace sdkffi::generator {

enum class FormatMethod
{
    kClangFormat,
    kJsonFormat,
    kXmlFormat
};
enum class FormatSetting
{
    kEnableOutputFormat,
    kDisableOutputFormat
};

class IOuputFormatter
{
protected:
    std::string file_name_;

public:
    IOuputFormatter(const std::string & file_name) :
        file_name_(file_name) {}

    virtual bool Format() = 0;
};

enum class PrototypeStyle
{
    kPascal,
    kCFamilly,
    kJson
};

struct ModuleInfo
{
    std::string name;
    std::string version;
    std::string description;
    std::string owner;

    std::string ToString() const
    {
        return fmt::format(
            "  name : {}\n"
            "  version  : {}\n"
            "  description : {}\n"
            "  owner : {}",
            name,
            version,
            description,
            owner);
    }
};

class IGenerator
{
protected:
    IOuputFormatter * formatter_;
    std::string       output_file_name_;

public:
    IGenerator(const std::string & output_file_name, IOuputFormatter * formatter) :
        output_file_name_(output_file_name), formatter_(formatter) {}

    virtual ModuleInfo GetInfo() const = 0;

    virtual bool Generate(FormatSetting format_setting) = 0;
};

}  // namespace sdkffi::generator