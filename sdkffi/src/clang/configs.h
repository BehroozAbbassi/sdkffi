#pragma once
namespace sdkffi::clang {

namespace configs {
enum class RecusriveVisitMode
{
    kDisable = 0,
    kEnable  = 1
};

struct LibclangArgs
{
    std::vector<std::string> clang_args;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(clang_args));
    }

    const char * const * GetArgs()
    {
        args_.resize(clang_args.size(), nullptr);

        std::transform(std::begin(clang_args), std::end(clang_args), std::begin(args_), [&](const std::string & str) { return str.c_str(); });

        return args_.data();
    }

    size_t GetArgsCount() const { return args_.size(); }

private:
    std::vector<const char *> args_ = {};
};

class LibclangConfigs
{
public:
    std::string  file_name;
    LibclangArgs clang_args;

    bool ReadFromFile(const std::string & file_name)
    {
        std::ifstream            ofs(file_name, std::ios::in);
        cereal::JSONInputArchive archive(ofs);

        archive(clang_args);

        return clang_args.clang_args.size() > 0;
    }
};
}  // namespace configs
inline void
PrintLibclangDiagnostics(CXTranslationUnit translationUnit)
{
    int nbDiag = clang_getNumDiagnostics(translationUnit);
    printf("There is %i diagnostics\n", nbDiag);

    for (unsigned int currentDiag = 0; currentDiag < nbDiag; ++currentDiag)
    {
        CXDiagnostic diagnotic = clang_getDiagnostic(translationUnit, currentDiag);
        CXString     errorString =
            clang_formatDiagnostic(diagnotic, clang_defaultDiagnosticDisplayOptions());

        auto message = std::string(clang_getCString(errorString));

        if (message.find(": warning:") != std::string::npos)
            PrintLogWarning(message);
        else if ((message.find(": fatal error:") != std::string::npos) ||
                 (message.find("error:") != std::string::npos))
            PrintLogError(message);

        // fprintf(stderr, "%s\n", clang_getCString(errorString));
        clang_disposeString(errorString);
    }
}

}  // namespace sdkffi::clang