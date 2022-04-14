#pragma once

#include "clang/libclang.h"

#include "types/data_types.h"

namespace sdkffi::parser::type {

struct CusrorAttributes
{
    CusrorAttributes(const CXCursor & cursor) :
        kind(clang_getCursorKind(cursor)),
        type(clang_getCursorType(cursor)),
        canonical_type(clang_getCanonicalType(clang_getCursorType(cursor))),
        parrent(clang_getCursorSemanticParent(cursor)),
        name(utils::string::CxStringToString(clang_getCursorSpelling(cursor))),
        is_anonymous(clang_Cursor_isAnonymous(cursor))

    {
    }

    CXCursorKind kind;
    CXType       type;
    CXType       canonical_type;
    std::string  name;
    CXCursor     parrent;
    bool         is_anonymous;
};

inline bool
IsCursorNestedType(const CXCursorKind & parent_cursor_kind)
{
    return (parent_cursor_kind == CXCursorKind::CXCursor_StructDecl ||
            parent_cursor_kind == CXCursorKind::CXCursor_UnionDecl ||
            parent_cursor_kind == CXCursorKind::CXCursor_EnumDecl);
}

inline bool
IsAnonymousNestedDecl(const CXCursor & cursor)
{
    CXCursor     parent             = clang_getCursorSemanticParent(cursor);
    CXCursorKind parent_cursor_kind = clang_getCursorKind(parent);
    // CXType       parent_cursor_type = clang_getCursorType(parent);

    if (clang_Cursor_isAnonymous(cursor) || clang_Cursor_isAnonymousRecordDecl(cursor))
    {
        if (parent_cursor_kind == CXCursorKind::CXCursor_StructDecl ||
            parent_cursor_kind == CXCursorKind::CXCursor_UnionDecl ||
            parent_cursor_kind == CXCursorKind::CXCursor_EnumDecl)
            return true;
    }

    return false;
}

enum class RecursiveMode
{
    kDisable,
    kEnable
};

struct Options
{
    CXTranslationUnit translation_unit;
    RecursiveMode     recusrive_mode;
};

class TypeParser
{
public:
    TypeParser(clang::LibclangState & libclang_state, const Options & options) :
        options_(options), libclang_state_(libclang_state)
    {
    }

    bool StartParsing()
    {
        PrintLogNormal("Start scanning functions ...");

        clang_visitChildren(libclang_state_.root_cursor,
                            &TypeParser::OnVisitor,
                            this);

        PrintLogNormal("finish scanning functions.");

        return true;
    }

    std::vector<types::Typedef> GetTypedefs() const { return typedefs_; }
    std::vector<types::Struct>  GetStructs() const { return structs_; }
    std::vector<types::Union>   GetUnions() const { return unions_; }
    std::vector<types::Enum>    GetEnums() const { return enums_; }

    std::vector<types::Typedef> & GetTypedefsByRef() { return typedefs_; }
    std::vector<types::Struct> &  GetStructsByRef() { return structs_; }
    std::vector<types::Union> &   GetUnionsByRef() { return unions_; }
    std::vector<types::Enum> &    GetEnumsByRef() { return enums_; }

    std::set<types::MacroDefine> GetStructDummyNames() const { return dummy_struct_names_; }
    std::set<types::MacroDefine> GetUnionDummyNames() const { return dummy_union_names_; }
    std::set<types::MacroDefine> GetEnumDummyNames() const { return dummy_enum_names_; }

private:
    Options                options_;
    clang::LibclangState & libclang_state_;

    types::Struct  struct_;
    types::Union   union_;
    types::Enum    enum_;
    types::Typedef typedef_;

    uint32_t struct_dummy_name_counter_ = 0;
    uint32_t union_dummy_name_counter_  = 0;

    std::vector<types::Typedef> typedefs_;
    std::vector<types::Struct>  structs_;
    std::vector<types::Union>   unions_;
    std::vector<types::Enum>    enums_;

    std::set<types::MacroDefine> dummy_struct_names_;
    std::set<types::MacroDefine> dummy_union_names_;
    std::set<types::MacroDefine> dummy_enum_names_;

private:
    bool IsCursorVisitable(const CXCursorKind & cursor_kind) const;

    CXChildVisitResult Visitor(CXCursor cursor, CXCursor parent);

    static CXChildVisitResult OnVisitor(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        return static_cast<TypeParser *>(data)->Visitor(cursor, parent);
    }

    // types::TypedefJm GetTypedefJm(const types::Typedef & typedf)
    //{
    //     if (typedf.type_kind.)
    //     {
    //     }
    // }

    types::Typedef GetTypedef(const CXCursor & cursor)
    {
        types::Typedef result = {};

        CusrorAttributes cursor_arrtibs(cursor);
        CusrorAttributes parrent_arrtibs(cursor_arrtibs.parrent);

        CXType cursor_type = clang_getCursorType(cursor);
        CXType type_name   = clang_getTypedefDeclUnderlyingType(cursor);

        auto type_name_cursor         = clang_getTypeDeclaration(type_name);
        auto type_name_pointee_type   = clang_getPointeeType(type_name);
        auto type_name_pointee_cursor = clang_getTypeDeclaration(type_name_pointee_type);
        // auto ct               = clang_getCanonicalType(type_name);
        // auto   ptr              = clang_getCursorReferenced(type_name_cursor);

        result.details.cursor = type_name_cursor;

        switch (type_name_cursor.kind)
        {
        case CXCursorKind::CXCursor_StructDecl:
            result.jm = types::TypedefJm::kStruct;
            break;
        case CXCursorKind::CXCursor_EnumDecl:
            result.jm = types::TypedefJm::kEnum;
            break;
        case CXCursorKind::CXCursor_UnionDecl:
            result.jm = types::TypedefJm::kUnion;
            break;
        case CXCursorKind::CXCursor_TypedefDecl:
            result.jm = types::TypedefJm::kTypedef;
            break;
        case CXCursorKind::CXCursor_FunctionDecl:
            break;
        default:
            result.jm = types::TypedefJm::kNone;

            break;
        }

        if (type_name.kind == CXTypeKind::CXType_Pointer)

        {
            result.details.cursor = type_name_pointee_cursor;
            switch (type_name_pointee_cursor.kind)
            {
            case CXCursorKind::CXCursor_StructDecl:
                result.jm = types::TypedefJm::kPtrToStruct;
                break;
            case CXCursorKind::CXCursor_EnumDecl:
                result.jm = types::TypedefJm::kPtrToEnum;
                break;
            case CXCursorKind::CXCursor_UnionDecl:
                result.jm = types::TypedefJm::kPtrToUnion;
                break;
            default:
                result.jm = types::TypedefJm::kNone;

                break;
            }
        }

        // std::string variable_type =
        // Libclang::Utils::CXTypeKindToString(type_name.kind);
        std::string variable_type =
            utils::string::CxStringToString(clang_getTypeKindSpelling(type_name.kind));

        result.alias             = utils::string::CxStringToString(clang_getCursorSpelling(cursor));
        result.type_kind         = variable_type;
        result.details.type_kind = type_name.kind;
        result.type_name         = utils::string::CxStringToString(clang_getTypeSpelling((type_name)));

        return result;
    }

    CXChildVisitResult        OnStructDeclVisitor(CXCursor cursor, CXCursor parent);
    static CXChildVisitResult OnStructDeclVisitor_Callback(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        return static_cast<TypeParser *>(data)->OnStructDeclVisitor(cursor, parent);
    }

    CXChildVisitResult        OnUnionDeclVisitor(CXCursor cursor, CXCursor parent);
    static CXChildVisitResult OnUnionDeclVisitor_Callback(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        return static_cast<TypeParser *>(data)->OnUnionDeclVisitor(cursor, parent);
    }

    CXChildVisitResult OnEnumDeclVisitor(CXCursor cursor, CXCursor parent)
    {
        types::Enum * result = &enum_;

        CXCursorKind cursor_kind = clang_getCursorKind(cursor);
        CXType       cursor_type = clang_getCursorType(cursor);

        std::string cursor_name =
            utils::string::CxStringToString(clang_getCursorSpelling(cursor));
        std::string data_tye =
            utils::string::CxStringToString(clang_getTypeSpelling(cursor_type));

        if (utils::libclang::IsAnonymousType(cursor))
            cursor_name = "Anonymous";

        types::EnumField enum_field = {};

        auto SignedValue   = clang_getEnumConstantDeclValue(cursor);
        auto UnsignedValue = clang_getEnumConstantDeclUnsignedValue(cursor);

        enum_field.name  = cursor_name;
        enum_field.value = std::to_string(SignedValue);
        // if (cursor_kind == CXCursor_FieldDecl)
        {
            result->fields.push_back(std::move(enum_field));
        }

        return CXChildVisitResult::CXChildVisit_Continue;
    }
    static CXChildVisitResult OnEnumDeclVisitor_Callback(CXCursor cursor, CXCursor parent, CXClientData data)
    {
        return static_cast<TypeParser *>(data)->OnEnumDeclVisitor(cursor, parent);
    }

    ///
    /// \brief Get name of elaborated nested Anonymous struct|union
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
    ///
    void GetNameOfElaboratedField(const CXCursor & cursor, types::GenericField & last_field);

    void onNestedStructDecl(CXCursor cursor, CXCursor parent)
    {
        types::Struct * struct_entry = &struct_;
        types::Union *  union_entry  = &union_;

        CusrorAttributes cursor_arrtibs(cursor);
        CusrorAttributes parrent_arrtibs(parent);

        types::Struct current_struct_def = struct_;
        types::Struct backup_struct      = struct_;

        struct_ = current_struct_def;

        std::string data_type = utils::string::CxStringToString(
            clang_getTypeSpelling(cursor_arrtibs.type));

        //
        // Nested Anonymous struct
        //

        if (clang_Cursor_isAnonymous(cursor) || clang_Cursor_isAnonymousRecordDecl(cursor))
        {
            struct_.attributes.is_anonymous = true;
            //
            // Use alias of parrent struct/union for nested anonymous members
            //
            if (IsAnonymousNestedDecl(cursor))
            {
                struct_.name = fmt::format("DUMMYSTRUCTNAME{0}", struct_dummy_name_counter_);  // parrent_cursor_name;

                dummy_struct_names_.insert({struct_.name, ""});

                struct_dummy_name_counter_++;
            }
            else
                struct_.name = "DUMMYSTRUCTNAME";
        }
        else
        {
            struct_.attributes.is_anonymous = false;
            struct_.name                    = cursor_arrtibs.name;
        }

        types::Struct tmp = *struct_entry;

        // if (IsCursorNestedType(parrent_arrtibs.kind))
        //    struct_entry->dyn_fields = {};

        // change curernt_struct_decl to the new nested struct

        struct_.dyn_fields = {};

        clang_visitChildren(cursor, &TypeParser::OnStructDeclVisitor_Callback, this);

        current_struct_def = struct_;
        struct_            = backup_struct;

        if (parrent_arrtibs.kind == CXCursorKind::CXCursor_StructDecl)
            struct_entry->dyn_fields.emplace_back(types::FieldType::kStruct, current_struct_def);

        else if (parrent_arrtibs.kind == CXCursorKind::CXCursor_UnionDecl)
            union_entry->dyn_fields.emplace_back(types::FieldType::kStruct, current_struct_def);
    }

    void onNestedUnionDecl(CXCursor cursor, CXCursor parent)
    {
        types::Struct * struct_entry = &struct_;
        types::Union *  union_entry  = &union_;

        CusrorAttributes cursor_arrtibs(cursor);
        CusrorAttributes parrent_arrtibs(parent);

        //
        // Nested Anonymous Union
        //
        if (clang_Cursor_isAnonymous(cursor) || clang_Cursor_isAnonymousRecordDecl(cursor))
        {
            union_.attributes.is_anonymous = true;
            //
            // Use alias of parrent struct/union for nested anonymous members
            //
            if (IsAnonymousNestedDecl(cursor))
            {
                if (union_dummy_name_counter_ >= 3)
                    auto bp = 1;

                union_.name = fmt::format("DUMMYUNIONNAME{0}", union_dummy_name_counter_);  // parrent_cursor_name;

                dummy_union_names_.insert({union_.name, ""});
                union_dummy_name_counter_++;
            }
            else
                union_.name = "DUMMYUNIONNAME";
        }
        else
        {
            union_.attributes.is_anonymous = false;
            union_.name                    = cursor_arrtibs.name;
        }

        types::Union current_struct_def = union_;
        types::Union backup_struct      = union_;

        union_            = current_struct_def;
        union_.dyn_fields = {};

        clang_visitChildren(cursor, &TypeParser::OnUnionDeclVisitor_Callback, this);

        current_struct_def = union_;
        union_             = backup_struct;

        // nested_union_def_level_--;

        //*struct_entry = tmp;

        if (parrent_arrtibs.kind == CXCursorKind::CXCursor_StructDecl)
            struct_entry->dyn_fields.emplace_back(types::FieldType::kUnion, current_struct_def);

        else if (parrent_arrtibs.kind == CXCursorKind::CXCursor_UnionDecl)
            union_entry->dyn_fields.emplace_back(types::FieldType::kUnion, current_struct_def);
    }

    types::StructField GetStructField(const CXCursor & cursor)
    {
        CusrorAttributes   cursor_arrtibs(cursor);
        std::string        data_type = utils::string::CxStringToString(clang_getTypeSpelling(cursor_arrtibs.type));
        types::StructField field;

        field.name = cursor_arrtibs.name;
        field.type = data_type;

        field.layout.offset    = clang_Cursor_getOffsetOfField(cursor);
        field.layout.alignment = clang_Type_getAlignOf(cursor_arrtibs.type);
        field.layout.size      = clang_Type_getSizeOf(cursor_arrtibs.type);
        field.layout.size      = clang_Type_getSizeOf(cursor_arrtibs.canonical_type);

        if (clang_Cursor_isBitField(cursor))
        {
            field.attributes.is_bit_field = true;
            field.bit_field               = clang_getFieldDeclBitWidth(cursor);
        }

        if (cursor_arrtibs.type.kind == CXTypeKind::CXType_ConstantArray)
        {
            std::string arr_type      = utils::string::CxStringToString(clang_getTypeSpelling(clang_getArrayElementType(cursor_arrtibs.type)));
            field.type                = arr_type;
            field.attributes.is_array = true;
            field.array_length        = clang_getArraySize(cursor_arrtibs.type);
        }

        return field;
    }

    void GetStruct(CXCursor cursor)
    {
        CusrorAttributes cursor_arrtibs(cursor);
        CusrorAttributes parrent_arrtibs(cursor_arrtibs.parrent);

        struct_dummy_name_counter_ = 0;
        union_dummy_name_counter_  = 0;

        struct_.details.cursor          = cursor;
        struct_.attributes.is_anonymous = cursor_arrtibs.is_anonymous;
        struct_.attributes.is_nested    = IsCursorNestedType(parrent_arrtibs.kind);
        struct_.name                    = cursor_arrtibs.name;

        clang_visitChildren(cursor, &TypeParser::OnStructDeclVisitor_Callback, this);

        structs_.push_back(std::move(struct_));
    }

    void GetUnion(CXCursor cursor)
    {
        CusrorAttributes cursor_arrtibs(cursor);
        CusrorAttributes parrent_arrtibs(cursor_arrtibs.parrent);

        struct_dummy_name_counter_ = 0;
        union_dummy_name_counter_  = 0;

        union_.attributes.is_anonymous = cursor_arrtibs.is_anonymous;
        union_.attributes.is_nested    = IsCursorNestedType(parrent_arrtibs.kind);
        union_.name                    = cursor_arrtibs.name;

        clang_visitChildren(cursor, &TypeParser::OnUnionDeclVisitor_Callback, this);

        unions_.push_back(union_);
    }
};
}  // namespace sdkffi::parser::type

namespace sdkffi::parser::dd {
inline bool
IsStructAlias(const types::Struct & st, const types::Typedef & typedf)
{
    if (typedf.jm != types::TypedefJm::kStruct && typedf.jm != types::TypedefJm::kPtrToStruct)
        return false;

    type::CusrorAttributes td_arrtibs(typedf.details.cursor);
    type::CusrorAttributes st_arrtibs(st.details.cursor);

    auto tt = clang_equalTypes(td_arrtibs.type, st_arrtibs.type);

    return tt;
}

inline std::vector<types::Typedef>
MapTypedefsToStruct(const std::vector<types::Typedef> & typedefs,
                    std::vector<types::Struct> &        structs)
{
    std::vector<types::Typedef> result   = {};
    std::vector<types::Struct>  structs1 = {};

    for (auto & st : structs)
    {
        for (const auto & td : typedefs)
        {
            if (IsStructAlias(st, td))
            {
                st.attributes.has_alias = true;
                st.alias_chain.push_back(td);
            }
        }
    }

    return result;
}

}  // namespace sdkffi::parser::dd