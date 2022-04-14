#include "pch.h"

#include "types/data_types.h"
#include "function_visitor.h"
#include "parser/type/type_visitor.h"

namespace sdkffi::parser::function {

CXChildVisitResult
Parser::Visitor(CXCursor cursor, CXCursor parent)
{
    if (options_.recusrive_mode == RecursiveMode::kDisable)
        if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
            return CXChildVisitResult::CXChildVisit_Continue;

    CXCursorKind cursor_kind = clang_getCursorKind(cursor);
    if (IsCursorFunction(cursor_kind) == false)
        return CXChildVisitResult::CXChildVisit_Continue;

    types::Function function = GetFunctionFromCursor(cursor);
    if (function.IsEmpty() == false)
    {
        function = GetAnnotatedFunction(function, cursor);
        functions_.push_back(std::move(function));
    }

    return CXChildVisitResult::CXChildVisit_Recurse;
}

bool
Parser::IsCursorFunction(const CXCursorKind & cursor_kind) const
{
    return (cursor_kind == CXCursorKind::CXCursor_FunctionDecl ||
            cursor_kind == CXCursorKind::CXCursor_CXXMethod ||
            cursor_kind == CXCursorKind::CXCursor_TypedefDecl ||
            cursor_kind == CXCursorKind::CXCursor_FunctionTemplate ||
            cursor_kind == CXCursorKind::CXCursor_Constructor) &&
           (cursor_kind != CXCursorKind::CXCursor_CallExpr);
}

std::string
Parser::CallingConventionToString(
    CXCallingConv calling_convention)
{
    switch (calling_convention)
    {
    case CXCallingConv::CXCallingConv_Default:
        return "Default";
    case CXCallingConv::CXCallingConv_C:
        return "__cdecl";
    case CXCallingConv::CXCallingConv_X86StdCall:
        return "__stdcall";
    case CXCallingConv::CXCallingConv_X86FastCall:
        return "__fastcall";
    case CXCallingConv::CXCallingConv_X86ThisCall:
        return "__thiscall";
    case CXCallingConv::CXCallingConv_X86Pascal:
        return "X86Pascal";
    case CXCallingConv::CXCallingConv_AAPCS:
        return "AAPCS";
    case CXCallingConv::CXCallingConv_AAPCS_VFP:
        return "AAPCS_VFP";
    case CXCallingConv::CXCallingConv_X86RegCall:
        return "X86RegCall";
    case CXCallingConv::CXCallingConv_IntelOclBicc:
        return "IntelOclBicc";
    case CXCallingConv::CXCallingConv_Win64:
        return "Win64";
        /* Alias for compatibility with older versions of API. */
        // case   CXCallingConv::CXCallingConv_X86_64Win64:
        //    return"";
        // case   CXCallingConv::CXCallingConv_Win64:
        //    return"";
    case CXCallingConv::CXCallingConv_X86_64SysV:
        return "X86_64SysV";
    case CXCallingConv::CXCallingConv_X86VectorCall:
        return "X86VectorCal";
    case CXCallingConv::CXCallingConv_Swift:
        return "Swift";
    case CXCallingConv::CXCallingConv_PreserveMost:
        return "PreserveMost";
    case CXCallingConv::CXCallingConv_PreserveAll:
        return "PreserveAll";
    case CXCallingConv::CXCallingConv_Invalid:
        return "Invalid";
    case CXCallingConv::CXCallingConv_Unexposed:
        return "Unexposed";
    }

    return "";
}
types::Function
Parser::GetFunctionFromCursor(CXCursor cursor)
{
    types::Function function_entry = {};
    types::Argument argument_entry = {};

    CXSourceRange cursor_range       = clang_getCursorExtent(cursor);
    CXType        cursor_type        = clang_getCursorType(cursor);
    CXCallingConv calling_convention = clang_getFunctionTypeCallingConv(cursor_type);

    //
    //
    //
    if (cursor_type.kind != CXTypeKind::CXType_FunctionProto &&
        cursor_type.kind != CXTypeKind::CXType_FunctionNoProto)
        return function_entry;

    function_entry.name               = utils::string::CxStringToString(clang_getCursorSpelling(cursor));
    function_entry.type               = utils::string::CxStringToString(clang_getTypeSpelling(clang_getResultType(cursor_type)));
    function_entry.calling_convention = CallingConventionToString(calling_convention);

    // auto func_tokens = lexer::TozenizeArgument(libclang_state_.translation_unit, cursor);
    // auto func_token  = std::find_if(func_tokens.begin(), func_tokens.end(), [&function_entry](const types::Token & tkn) {
    //     return (function_entry.name == tkn.value);
    // });

    // if (func_token != func_tokens.end())
    //     function_entry.range = (*func_token).range;
    // else
    //     auto bp = 1;

    int nameless_count  = 0;
    int arguments_count = clang_Cursor_getNumArguments(cursor);

    for (int arg_index = 0; arg_index < arguments_count; ++arg_index)
    {
        argument_entry.attributes.is_name_less = false;

        CXCursor      arg_cursor = clang_Cursor_getArgument(cursor, arg_index);
        CXSourceRange arg_range  = clang_getCursorExtent(arg_cursor);
        std::string   arg_name   = utils::string::CxStringToString(clang_getCursorSpelling(arg_cursor));

        if (arg_name.empty())
        {
            arg_name                               = "namelessArg_" + std::to_string(nameless_count);
            argument_entry.attributes.is_name_less = true;
            nameless_count++;
        }

        argument_entry.tokens = lexer::TozenizeArgument(libclang_state_.translation_unit, arg_cursor);
        argument_entry.name   = arg_name;
        argument_entry.type   = utils::string::CxStringToString(
            clang_getTypeSpelling(clang_getArgType(cursor_type, arg_index)));
        // argument_entry.to = arg_range;

        function_entry.arguments.push_back(argument_entry);
        argument_entry = {};
    }

    return function_entry;
}
types::Function
Parser::GetAnnotatedFunction(const types::Function & func, CXCursor cursor)
{
    types::Function function_entry = func;

    if (func.name == "is_wctype")
    {
        auto bp = 1;
        std::cerr << func.name;
    }

    const auto & tokens = lexer::TokenizeCursor(libclang_state_.translation_unit, cursor);

    if (tokens.size() <= 0)
        return func;

    auto func_token_find = std::find_if(tokens.begin(), tokens.end(), [&function_entry](const types::Token & tkn) {
        // auto is_same_range = std::tie(function_entry.range.begin_int_data, function_entry.range.end_int_data) ==
        //                      std::tie(tkn.range.begin_int_data, tkn.range.end_int_data);
        // return is_same_range && (function_entry.name == tkn.value);
        return function_entry.name == tkn.value;
    });

    if (func_token_find == tokens.end())
        return func;

    auto func_name_pos = func_token_find - tokens.begin();

    uint32_t prev_arg_pos = func_name_pos;
    uint32_t arg_index    = 0;

    for (auto & arg : function_entry.arguments)
    {
        auto curr_arg_pos = lexer::GetArgumentTokenIndex(arg, tokens);
        if (curr_arg_pos == -1)
            continue;

        {
            uint32_t delim_pos = prev_arg_pos + 2;

            auto sal_begin = delim_pos;
            auto sal_end   = (curr_arg_pos - arg.tokens.size()) + 1;

            if (arg.attributes.is_name_less == true /*&& function_entry.Parameters.size() == 1*/)
                sal_end -= 2;

            if (sal_begin > sal_end || sal_end > tokens.size())
                continue;

            std::vector<types::Token> sal_vec(&tokens[sal_begin], &tokens[sal_end]);
            std::for_each(sal_vec.begin(), sal_vec.end(), [&](const types::Token & tkn) {
                //
                // Filter inline comments in the function prototype
                //
                if (tkn.value[0] != '/' && tkn.value[1] != '/')
                    arg.annotation += tkn.value;
            });
        }

        // if (arg_index == 0)
        //{
        //     //prev_arg_pos = curr_arg_pos;
        //     // point after first open "(" in prototype
        //     uint32_t delim_pos = func_name_pos + 2;

        //    auto sal_begin = delim_pos;
        //    auto sal_end   = (curr_arg_pos - arg.tokens.size()) + 1;

        //    if (arg.attributes.is_name_less ==
        //        true /*&& function_entry.Parameters.size() == 1*/)
        //        sal_end - 2;

        //    if (sal_begin > sal_end || sal_end > tokens.size())
        //        continue;

        //    std::vector<types::Token> sal_vec(&tokens[sal_begin], &tokens[sal_end]);
        //    std::for_each(sal_vec.begin(), sal_vec.end(), [&](const types::Token & tkn) {
        //        //
        //        // Filter inline comments in the function prototype
        //        //
        //        if (tkn.value[0] != '/' && tkn.value[1] != '/')
        //            arg.annotation += tkn.value;
        //    });
        //}
        // else if (arg_index >= 1)
        //{
        //    auto sal_begin = prev_arg_pos + 2;  // delim_pos;
        //    auto sal_end   = (curr_arg_pos - arg.tokens.size()) + 1;

        //    if (sal_begin > sal_end || sal_end > tokens.size())
        //        break;

        //    std::vector<types::Token> sal_vec(&tokens[sal_begin], &tokens[sal_end]);
        //    std::for_each(sal_vec.begin(), sal_vec.end(), [&](const types::Token & tkn) {
        //        //
        //        // Filter inline comments in the function prototype
        //        //
        //        if (tkn.value[0] != '/' && tkn.value[1] != '/')
        //            arg.annotation += tkn.value;
        //    });
        //}
        prev_arg_pos = curr_arg_pos;
        arg_index++;
    }

    return function_entry;
}
}  // namespace sdkffi::parser::function