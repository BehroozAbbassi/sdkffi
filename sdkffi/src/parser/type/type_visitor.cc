#include "pch.h"
#include "type_visitor.h"

namespace sdkffi::parser::type {
bool
TypeParser::IsCursorVisitable(const CXCursorKind & cursor_kind) const
{
    switch (cursor_kind)
    {
    case CXCursorKind::CXCursor_TypedefDecl:
    case CXCursorKind::CXCursor_StructDecl:
    case CXCursorKind::CXCursor_UnionDecl:
    case CXCursorKind::CXCursor_EnumDecl:
    case CXCursorKind::CXCursor_MacroExpansion:
        return true;
    default:
        break;
    }

    return false;
}

CXChildVisitResult
TypeParser::Visitor(CXCursor cursor, CXCursor parent)
{
    if (options_.recusrive_mode == RecursiveMode::kDisable)
        if (clang_Location_isFromMainFile(clang_getCursorLocation(cursor)) == 0)
            return CXChildVisitResult::CXChildVisit_Continue;

    CusrorAttributes cursor_arrtibs(cursor);
    CusrorAttributes parrent_arrtibs(parent);

    //
    // Validations
    //
    if (utils::libclang::IsForwardDeclaration(cursor) || IsCursorVisitable(cursor_arrtibs.kind) == false)
        return CXChildVisitResult::CXChildVisit_Continue;

    //
    // Clear global (in class scope) variables for new iterations
    //
    struct_  = {};
    union_   = {};
    typedef_ = {};
    enum_    = {};

    switch (cursor_arrtibs.kind)
    {
    case CXCursorKind::CXCursor_TypedefDecl:
        typedef_ = GetTypedef(cursor);
        typedefs_.push_back(typedef_);
        break;

    case CXCursorKind::CXCursor_StructDecl:
        GetStruct(cursor);
        break;

    case CXCursorKind::CXCursor_UnionDecl:
        GetUnion(cursor);
        break;

    case CXCursorKind::CXCursor_EnumDecl:
        enum_.name = cursor_arrtibs.name;
        clang_visitChildren(cursor, &TypeParser::OnEnumDeclVisitor_Callback, this);

        enums_.push_back(enum_);
        break;

    case CXCursorKind::CXCursor_MacroExpansion:
        // std::cout << "Location is ";
        // printLocation(clang_getCursorLocation(cursor));
        // std::cout << ". ";
        // printExtent(clang_getCursorExtent(cursor));
        // printExtent(clang_getCursorExtent(parent));
        // std::cout << "\n";
        // clang_tokenize()
    default:
        break;
    }

    return CXChildVisitResult::CXChildVisit_Continue;
    return CXChildVisitResult::CXChildVisit_Recurse;
}

CXChildVisitResult
TypeParser::OnStructDeclVisitor(CXCursor cursor, CXCursor parent)
{
    types::Struct * struct_entry       = &struct_;
    types::Struct   current_struct_def = struct_;

    CusrorAttributes cursor_arrtibs(cursor);
    CusrorAttributes parrent_arrtibs(parent);

    //
    // Visiting struct fields
    //
    if (parrent_arrtibs.kind != CXCursorKind::CXCursor_StructDecl)
        return CXChildVisitResult::CXChildVisit_Continue;

    std::string parrent_cursor_data_type = utils::string::CxStringToString(
        clang_getTypeSpelling(parrent_arrtibs.type));

    std::string data_type = utils::string::CxStringToString(
        clang_getTypeSpelling(cursor_arrtibs.type));

    //
    // Normal struct fields
    //

    if (cursor_arrtibs.kind == CXCursorKind::CXCursor_FieldDecl)
    {
        if (cursor_arrtibs.type.kind == CXTypeKind::CXType_Elaborated)
        {
            if (struct_entry->dyn_fields.empty() == false)
            {
                auto & last_field = struct_entry->dyn_fields.back();
                GetNameOfElaboratedField(cursor, last_field);
            }

            // auto                         x     = cursor_arrtibs.name;
            // models::type::StructFieldDef field = GetStructField(cursor);
            //  result->dyn_fields.emplace_back(models::type_kind::TypeKind::kStructField,
            //  field); return CXChildVisitResult::CXChildVisit_Continue;
        }
        else if (cursor_arrtibs.type.kind != CXTypeKind::CXType_Elaborated)
        {
            types::StructField field = GetStructField(cursor);

            struct_entry->dyn_fields.emplace_back(
                types::FieldType::kStructField,
                field);
            // return CXChildVisitResult::CXChildVisit_Continue;
        }
    }

    //
    // Add nested Struct or Unions to visited structs
    //

    if (cursor_arrtibs.kind == CXCursorKind::CXCursor_StructDecl)
    {
        onNestedStructDecl(cursor, parent);
    }
    else if (cursor_arrtibs.kind == CXCursorKind::CXCursor_UnionDecl)
    {
        onNestedUnionDecl(cursor, parent);
    }

    return CXChildVisitResult::CXChildVisit_Continue;
    return CXChildVisitResult::CXChildVisit_Recurse;
}

/// \brief Get name of elaborated nested Anonymous struct|union
///
///
///
/// \code
///
/// struct|union {
///    struct|union {
///    ...
///    } some_name; // Elaborated && Record
///
///    int normal_member;
///
///    struct|union {
///        int int_member;
///    }; // nested nameless struct|union
/// }
///
/// \endcode
///
/// \param cursor [in]
/// \param last_field [in/out]

inline void
TypeParser::GetNameOfElaboratedField(const CXCursor & cursor, types::GenericField & last_field)
{
    // if (IsAnonymousNestedDecl(cursor) == false)
    //     return;

    CusrorAttributes cursor_arrtibs(cursor);
    CusrorAttributes parrent_arrtibs(cursor_arrtibs.parrent);

    if (cursor_arrtibs.name.empty())
        return;  // no name :(

    if (cursor_arrtibs.kind != CXCursorKind::CXCursor_FieldDecl)
        return;

    if (cursor_arrtibs.type.kind != CXTypeKind::CXType_Elaborated ||
        parrent_arrtibs.type.kind != CXTypeKind::CXType_Record)
        return;

    std::string data_type = utils::string::CxStringToString(
        clang_getTypeSpelling(cursor_arrtibs.type));

    //
    // TODO: resolve typedef type_names
    //
    if (data_type.find("struct") != std::string::npos ||
        data_type.find("union") != std::string::npos)
    {
        if (last_field.type == types::FieldType::kStruct)
        {
            auto & st = std::any_cast<types::Struct &>(last_field.value);

            if (st.instance_name.empty())
                st.instance_name = cursor_arrtibs.name;
        }
        else if (last_field.type == types::FieldType::kUnion)
        {
            auto & st = std::any_cast<types::Union &>(last_field.value);

            if (st.instance_name.empty())
                st.instance_name = cursor_arrtibs.name;
        }
    }
}
CXChildVisitResult
TypeParser::OnUnionDeclVisitor(CXCursor cursor, CXCursor parent)
{
    types::Union * union_entry = &union_;

    CXCursorKind cursor_kind = clang_getCursorKind(cursor);
    CXType       cursor_type = clang_getCursorType(cursor);

    std::string cursor_name =
        utils::string::CxStringToString(clang_getCursorSpelling(cursor));
    std::string data_type =
        utils::string::CxStringToString(clang_getTypeSpelling(cursor_type));

    //
    // Add nested Struct or Unions to visited structs
    //
    CXCursor     parrent_cursor      = clang_getCursorSemanticParent(cursor);
    CXCursorKind parrent_cursor_kind = clang_getCursorKind(parrent_cursor);
    CXType       parrent_cursor_type = clang_getCursorType(parrent_cursor);
    std::string  parrent_cursor_name =
        utils::string::CxStringToString(clang_getCursorSpelling(parrent_cursor));
    std::string parrent_cursor_data_type = utils::string::CxStringToString(
        clang_getTypeSpelling(parrent_cursor_type));

    CusrorAttributes cursor_arrtibs(cursor);
    CusrorAttributes parrent_arrtibs(parent);

    // std::cout << std::string(nested_union_def_level_, '+')
    //           << std::string(union_def_level_, '.') << "OnUnionDeclVisitor "
    //           << struct_visit_depth_ << "  "
    //           << "  " << nested_union_def_level_ << "  " << cursor_arrtibs.name
    //           << "\n";
    //
    //  Add nested Struct or Unions to visited structs
    //

    if (cursor_kind == CXCursorKind::CXCursor_UnionDecl)
    {
        onNestedUnionDecl(cursor, parent);
    }
    else if (cursor_kind == CXCursorKind::CXCursor_StructDecl)
    {
        onNestedStructDecl(cursor, parent);
    }

    //
    // Normal fields
    //

    if (cursor_arrtibs.kind == CXCursorKind::CXCursor_FieldDecl)
    {
        if (cursor_arrtibs.type.kind == CXTypeKind::CXType_Elaborated)
        {
            if (union_entry->dyn_fields.empty() == false)
            {
                auto & last_field = union_entry->dyn_fields.back();
                GetNameOfElaboratedField(cursor, last_field);
            }
        }
        else if (cursor_type.kind != CXTypeKind::CXType_Elaborated)
        {
            if (utils::libclang::IsAnonymousType(cursor))
                cursor_name = "Anonymous";

            types::UnionField union_field = {cursor_name, data_type};

            if (clang_Cursor_isBitField(cursor))
            {
                union_field.attributes.is_bit_field = true;
                union_field.bit_field               = clang_getFieldDeclBitWidth(cursor);
            }

            if (cursor_arrtibs.type.kind == CXTypeKind::CXType_ConstantArray)
            {
                std::string arr_type            = utils::string::CxStringToString(clang_getTypeSpelling(clang_getArrayElementType(cursor_arrtibs.type)));
                union_field.type                = arr_type;
                union_field.attributes.is_array = true;
                union_field.array_length        = clang_getArraySize(cursor_arrtibs.type);
            }

            union_entry->dyn_fields.emplace_back(types::FieldType::kUnionField,
                                                 union_field);
        }
    }

    return CXChildVisitResult::CXChildVisit_Continue;
    return CXChildVisitResult::CXChildVisit_Recurse;
}

}  // namespace sdkffi::parser::type