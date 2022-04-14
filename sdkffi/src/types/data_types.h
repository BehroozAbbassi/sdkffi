#pragma once

namespace sdkffi::types {
namespace detail {

template <typename T>
class Visitor
{
public:
    virtual void Visit(T & visitable) = 0;
};

template <typename T>
class Visitable
{
public:
    virtual void Accept(Visitor<T> & visitor) = 0;
};

}  // namespace detail

struct Token
{
    CXSourceRange range;
    std::string   value;

    inline bool operator==(const Token & rhs) const
    {
        auto range_t = std::tie(range.begin_int_data,
                                range.end_int_data) == std::tie(rhs.range.begin_int_data,
                                                                rhs.range.end_int_data);
        return (range_t && (value == rhs.value));
    }
};

struct Inclusion
{
    std::string what;
    std::string from;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(what),
                CEREAL_NVP(from));
    }
};

struct Variable
{
    std::string name;
    std::string type;
};

struct MacroDefine
{
    std::string name;
    std::string value;

    inline bool operator<(const MacroDefine & rhs) const
    {
        return name < rhs.name;
        return (std::tie(value, rhs.value) < std::tie(name, rhs.name));
    }

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(value));
    }
};

typedef std::uint64_t TypeId;
typedef std::uint32_t FiledOrder;

enum TypeUsageRelation
{
    kUsedBy,
    kUsed
};

struct TypeRelation
{
    FiledOrder order;
    TypeId     to;
};

enum class TypedefJm
{
    kNone,

    kStruct,
    kEnum,
    kUnion,

    kPtrToStruct,
    kPtrToEnum,
    kPtrToUnion,

    kPointer,
    kTypedef
};

struct Typedef
{
    std::string alias;
    std::string type_kind;
    std::string type_name;

    std::vector<Typedef> chain;
    TypedefJm            jm;

    //
    // Internal usage of libclang meta data
    //
    struct ClangInternal
    {
        // points to type name curser
        CXCursor   cursor;
        CXCursor   type_name_cursor;
        CXTypeKind type_kind;
    } details;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(alias),
                CEREAL_NVP(type_kind),
                CEREAL_NVP(type_name));
    }

    inline bool operator<(const Typedef & rhs) const
    {
        return (alias < rhs.alias);
        return ((alias < rhs.alias) && (type_name < rhs.type_name));
    }
};

struct FiledLayout
{
    size_t offset;
    size_t size;
    size_t alignment;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(offset),
                CEREAL_NVP(size),
                CEREAL_NVP(alignment));
    }
    // clang_Type_getAlignOf
    // clang_Type_getSizeOf
    // clang_Type_getOffsetOf
    // clang_Cursor_isBitField
};

enum struct FieldType
{
    kNone,
    kStruct,
    kStructField,
    kUnion,
    kUnionField,
    kEnum,
    kEnumField,
    kVariable,
    kTypedef,
    kBuiltin
};

struct GenericField
{
    FieldType type;
    std::any  value;

    GenericField() = default;
    GenericField(FieldType type, std::any value) :
        type(type), value(value)
    {
    }
};

struct EnumField
{
    std::string name;
    std::string value;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(value));
    }
};

struct Enum
{
    std::string            name;
    std::vector<EnumField> fields;

    struct Attribute
    {
        bool is_anonymous;

        template <class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(is_anonymous));
        }
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(fields),
                CEREAL_NVP(attributes));
    }
};

struct UnionField
{
    std::string name;
    std::string type;
    FiledLayout layout;

    std::uint32_t bit_field;
    std::uint32_t array_length;

    FieldType    field_kind;
    TypeRelation relation;

    struct Attribute
    {
        bool is_bit_field = false;
        bool is_array     = false;

        template <class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(is_bit_field),
                    CEREAL_NVP(is_array));
        }
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(type),
                CEREAL_NVP(layout),
                CEREAL_NVP(attributes),
                CEREAL_NVP(bit_field),
                CEREAL_NVP(array_length));
    }
};

struct Union
{
    std::string               name;
    std::string               typedef_name;
    std::string               instance_name;
    std::vector<GenericField> dyn_fields;

    struct Attribute
    {
        bool is_nested;
        bool is_anonymous;

        template <class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(is_nested),
                    CEREAL_NVP(is_anonymous));
        }
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(typedef_name));
    }
};

struct StructField : public detail::Visitable<StructField>
{
    std::string   name;
    std::string   type;
    std::uint32_t bit_field;
    std::uint32_t array_length;
    FiledLayout   layout;

    FieldType    field_kind;
    TypeRelation relation;

    struct Attribute
    {
        bool is_bit_field = false;
        bool is_array     = false;

        template <class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(is_bit_field),
                    CEREAL_NVP(is_array));
        }
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(type),
                CEREAL_NVP(bit_field),
                CEREAL_NVP(array_length),
                CEREAL_NVP(layout),
                CEREAL_NVP(attributes));
    }

    void Accept(detail::Visitor<StructField> & visitor) override
    {
        visitor.Visit(static_cast<StructField &>(*this));
    }

    // inline bool operator<(const StructFieldDef & rhs) const
    //{
    //     return (std::tie(name, type, bit_field) < std::tie(rhs.name, rhs.type, rhs.bit_field));
    // }
};

struct Struct : public detail::Visitable<Struct>
{
    TypeId                    id;
    std::string               name;
    std::string               instance_name;
    std::vector<GenericField> dyn_fields;
    std::vector<Typedef>      alias_chain;

    struct ClangInternal
    {
        CXCursor cursor;
    } details;

    std::vector<StructField>  fileds;
    std::vector<TypeRelation> relations;

    struct Attribute
    {
        bool is_nested     = false;
        bool is_anonymous  = false;
        bool has_bit_field = false;
        bool has_alias     = false;
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                // CEREAL_NVP(dyn_fields),
                CEREAL_NVP(instance_name),
                CEREAL_NVP(attributes));
    }

    void Accept(detail::Visitor<Struct> & visitor) override
    {
        visitor.Visit(static_cast<Struct &>(*this));
    }
};

struct TypesGraph
{
    std::vector<types::Typedef> typedefs;
    std::vector<types::Struct>  structs;
    std::vector<types::Union>   unions;
    std::vector<types::Enum>    enums;

    std::set<types::MacroDefine> dummy_struct_names;
    std::set<types::MacroDefine> dummy_union_names;
    std::set<types::MacroDefine> dummy_enum_names;
};

struct Argument
{
    std::string        name;
    std::string        type;
    std::string        annotation;
    std::vector<Token> tokens;

    struct Attribute
    {
        bool is_name_less;

        template <class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(is_name_less));
        }
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(type),
                CEREAL_NVP(annotation),
                CEREAL_NVP(attributes));
    }

    inline bool operator==(const Argument & rhs) const
    {
        return ((rhs.name == name) && (rhs.type == type));
    }
};

struct Function
{
    std::string           name;
    std::string           type;
    std::string           calling_convention;
    std::vector<Argument> arguments;
    CXSourceRange         range;
    std::vector<Token>    tokens;

    struct FunctionAttribute
    {
        bool is_anonymous;
        template <class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(is_anonymous));
        }
    } attributes;

    template <class Archive>
    void serialize(Archive & archive)
    {
        archive(CEREAL_NVP(name),
                CEREAL_NVP(type),
                CEREAL_NVP(calling_convention),
                CEREAL_NVP(arguments));
    }

    bool IsEmpty() const
    {
        return (name.empty() && type.empty() && calling_convention.empty() && arguments.size() <= 0);
    }
};

}  // namespace sdkffi::types