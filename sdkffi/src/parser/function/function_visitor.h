#pragma once
#include "clang/libclang.h"

namespace sdkffi::parser::lexer {
// inline std::string GetCursorFileLocation(const CXCursor & cursor)
//{
//    CXFile file;
//    unsigned int line = 0, col = 0, offset = 0;
//    clang_getSpellingLocation(clang_getCursorLocation(cursor), &file, &line,
//    &col, &offset);
//}
inline std::string
getClangFileName(const CXFile & file)
{
    std::string filename;

    CXString cxfilename = clang_getFileName(file);
    if (clang_getCString(cxfilename) != 0)
        filename = clang_getCString(cxfilename);
    clang_disposeString(cxfilename);

    return "\"" + filename + "\"";
}

/////////////////////////////////////////////////////
inline void
printLocation(CXSourceLocation location)
{
    CXFile       file;
    unsigned int line = 0, col = 0, offset = 0;
    clang_getSpellingLocation(location, &file, &line, &col, &offset);
    std::cout << line << ", " << col << ", " << offset << ", "
              << getClangFileName(file);
}

/////////////////////////////////////////////////////
///

struct TokenEntry
{
    CXSourceLocation location;
    std::string      Value;
};

inline uint32_t
GetArgumentTokenIndex(const types::Argument &           arg,
                      const std::vector<types::Token> & tokens)
{
    uint32_t index = 0;

    for (auto && token : tokens)
    {
        if (arg.tokens.size() == 1)
        {
            if (arg.attributes.is_name_less)
            {
                if (token.value == arg.type)
                    return index;
            }
            else
            {
                if (token.value == arg.name)
                    return index;
            }
        }
        else
        {
            uint32_t arg_index   = 0;
            size_t   token_index = index;

            if (arg.tokens.size() == 0)
                return -1;

            if (token == arg.tokens.front())
            {
                std::vector<bool> found = {};
                for (auto && arg_token : arg.tokens)
                {
                    if (token_index <= tokens.size())
                    {
                        if (tokens.at(token_index) == arg_token)
                            found.push_back(true);
                        else
                            found.push_back(false);

                        token_index++;
                    }
                }

                if (std::adjacent_find(found.begin(), found.end(), std::not_equal_to<>()) == found.end())
                    return token_index - 1;
            }
        }

        index++;
    }
    return -1;
}
inline void
printExtent(CXSourceRange range)
{
    std::cout << "Extent begin at location [";
    printLocation(clang_getRangeStart(range));
    std::cout << "]. \n";
    std::cout << "Extent finish at location [";
    printLocation(clang_getRangeEnd(range));
    std::cout << "]. \n";
}

inline std::vector<types::Token>
TozenizeArgument(CXTranslationUnit & translation_unit, const CXCursor & cursor)
{
    std::vector<types::Token> result = {};

    CXToken *     tokens       = {};
    CXSourceRange cursor_range = clang_getCursorExtent(cursor);
    uint32_t      token_count  = 0;

    clang_tokenize(translation_unit, cursor_range, &tokens, &token_count);

    for (int i = 0; i < token_count; ++i)
    {
        types::Token token_entry = {};
        CXString     cxtoken     = clang_getTokenSpelling(translation_unit, tokens[i]);

        const auto token = tokens[i];

        CXTokenKind      token_kind     = clang_getTokenKind(token);
        CXString         token_spelling = clang_getTokenSpelling(translation_unit, token);
        CXSourceLocation token_location = clang_getTokenLocation(translation_unit, token);
        CXSourceRange    token_range    = clang_getTokenExtent(translation_unit, token);

        token_entry.range = token_range;
        token_entry.value = clang_getCString(cxtoken);

        result.push_back(std::move(token_entry));

        clang_disposeString(cxtoken);
    }

    clang_disposeTokens(translation_unit, tokens, token_count);

    return result;
}

inline std::vector<types::Token>
TokenizeCursor(CXTranslationUnit & translation_unit,
               const CXCursor &    cursor)
{
    std::vector<types::Token> result = {};

    CXToken *     tokens       = {};
    CXSourceRange cursor_range = clang_getCursorExtent(cursor);
    uint32_t      token_count  = 0;

    CXSourceLocation l = clang_getCursorLocation(cursor);
    CXFile           curr_file;
    clang_getExpansionLocation(l, &curr_file, nullptr, nullptr, nullptr);

    std::string curr_file_name;
    std::string included_file_name;

    if (curr_file != nullptr)
    {
        curr_file_name =
            utils::string::CxStringToString(clang_getFileName(curr_file));
    }

    //
    // Tokenize the whole translation unit
    //

    // CXSourceRange range2 = clang_getCursorExtent(clang_getTranslationUnitCursor(translation_unit));
    // printExtent(cursor_range);
    ////clang_annotateTokens
    // auto s = 2;
    // if (s == 1)
    //     clang_tokenize(translation_unit, range2, &tokens, &token_count);

    //
    // Tokenize current cursor
    //
    clang_tokenize(translation_unit, cursor_range, &tokens, &token_count);
    for (int i = 0; i < token_count; ++i)
    {
        const CXToken & token = tokens[i];

        types::Token token_entry = {};

        CXTokenKind      token_kind     = clang_getTokenKind(token);
        CXString         token_spelling = clang_getTokenSpelling(translation_unit, token);
        CXSourceLocation token_location = clang_getTokenLocation(translation_unit, token);
        CXSourceRange    token_range    = clang_getTokenExtent(translation_unit, token);

        token_entry.range = token_range;
        token_entry.value = utils::string::CxStringToString(token_spelling);

        result.push_back(std::move(token_entry));
    }

    clang_disposeTokens(translation_unit, tokens, token_count);

    return result;
}

}  // namespace sdkffi::parser::lexer

namespace sdkffi::parser::function {

enum class RecursiveMode
{
    kDisable,
    kEnable
};

enum class ParsingStatus
{
    kNone,
    kRunning,
    kFinished
};

struct Options
{
    CXTranslationUnit translation_unit;
    RecursiveMode     recusrive_mode;
};

class Parser
{
public:
    Parser(clang::LibclangState & libclang_state, const Options & options) :
        options_(options), libclang_state_(libclang_state)
    {
    }

    bool StartParsing()
    {
        PrintLogNormal("Start scanning functions ...");

        status_ = ParsingStatus::kRunning;
        clang_visitChildren(libclang_state_.root_cursor,
                            &Parser::OnVisitor,
                            this);
        status_ = ParsingStatus::kFinished;

        PrintLogNormal("finish scanning functions.");

        return true;
    }

    ParsingStatus                  GetStatus() const { return status_; }
    std::vector<types::Function>   GetFunctions() const { return functions_; }
    std::vector<types::Function> & GetFunctionsByRef() { return functions_; }

private:
    std::vector<types::Function> functions_;
    Options                      options_;
    ParsingStatus                status_ = ParsingStatus::kNone;
    clang::LibclangState &       libclang_state_;

private:
    bool            IsCursorFunction(const CXCursorKind & cursor_kind) const;
    std::string     CallingConventionToString(CXCallingConv calling_convention);
    types::Function GetFunctionFromCursor(CXCursor cursor);

    types::Function GetAnnotatedFunction(const types::Function & func, CXCursor cursor);

    CXChildVisitResult Visitor(CXCursor cursor, CXCursor parent);

    static CXChildVisitResult OnVisitor(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        return static_cast<Parser *>(data)->Visitor(cursor, parent);
    }
};
}  // namespace sdkffi::parser::function