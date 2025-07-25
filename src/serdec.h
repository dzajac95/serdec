// This is the "base" of Serdec which only parses a small subset of C which then
// can be used by specific generators like serdec_jim.h and serdec_debug.h.
#ifndef SERDEC_H_
#define SERDEC_H_

#include "stb_c_lexer.h"
#define NOB_STRIP_PREFIX
#include "nob.h"

typedef enum {
    TYPE_STRING,
    TYPE_DOUBLE,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_STRUCT,
    COUNT_TYPES,
} Type_Kind;

typedef enum {
    STRUCT_OBJECT,
    STRUCT_ARRAY,
    COUNT_STRUCTS,
} Struct_Kind;

typedef struct {
    Type_Kind kind;
    const char *typename;
} Type;

typedef struct {
    const char *name;
    const char *tag;
    Type type;
} Field;

typedef struct {
    Field *items;
    size_t count;
    size_t capacity;
} Object;

typedef struct {
    Type type;
} Array;

typedef union {
    Object object;
    Array array;
} Struct_As;

typedef struct {
    const char *name;
    Struct_Kind kind;
    Struct_As as;
} Struct;

typedef struct {
    Struct *items;
    size_t count;
    size_t capacity;
} Structs;

typedef struct {
    stb_lexer l;
    const char *file_path;
    char string_store[1024];
    Cmd parsed_structs;
} Serdec;

void serdec_init(Serdec *serdec, const char *file_path, String_Builder file_data);
void dump_struct(Struct strukt);
bool expect_token(Serdec *serdec, long token);
bool get_and_expect_token(Serdec *serdec, long token);
bool get_and_expect_id(Serdec *serdec, const char *name);
bool parse_type(Serdec *serdec, Type *type);
bool parse_field(Serdec *serdec, Field *field);
bool parse_struct(Serdec *serdec, Struct *strukt);
bool parse_structs(Serdec *serdec, Structs *structs);
bool struct_parsed(Serdec *serdec, const char *name);

#endif // SERDEC_H_

#ifdef SERDEC_IMPLEMENTATION

void serdec_init(Serdec *serdec, const char *file_path, String_Builder file_data)
{
    stb_c_lexer_init(&serdec->l, file_data.items, file_data.items + file_data.count, serdec->string_store, sizeof(serdec->string_store));
    serdec->file_path = file_path;
}

bool struct_parsed(Serdec *serdec, const char *name)
{
    da_foreach(const char*, s, &serdec->parsed_structs) {
        if (strcmp(*s, name) == 0) return true;
    }
    return false;
}

void print_type(Type type)
{
    switch (type.kind) {
    case TYPE_STRING: printf("string"); break;
    case TYPE_DOUBLE: printf("double"); break;
    case TYPE_INT:    printf("int"); break;
    case TYPE_BOOL:   printf("bool"); break;
    case TYPE_STRUCT: printf("%s", type.typename); break;
    case COUNT_TYPES: default: UNREACHABLE("type.kind");
    }
    printf("\n");
}

void dump_struct(Struct strukt)
{
    printf("struct %s:\n", strukt.name);
    printf("kind: %d\n", strukt.kind);
    switch (strukt.kind) {
    case STRUCT_OBJECT:
        da_foreach(Field, field, &strukt.as.object) {
            printf("    %s: ", field->name);
            print_type(field->type);
        }
        break;
    case STRUCT_ARRAY:
        printf("Array type:");
        print_type(strukt.as.array.type);
        break;
    case COUNT_STRUCTS: default: UNREACHABLE("strukt.kind");
    }
}

bool expect_token(Serdec *serdec, long token)
{
    if (serdec->l.token != token) {
        stb_lex_location loc = {0};
        stb_c_lexer_get_location(&serdec->l, serdec->l.where_firstchar, &loc);
        fprintf(stderr, "%s:%d:%d: ERROR: expected %ld, but got %ld\n", serdec->file_path, loc.line_number, loc.line_offset, token, serdec->l.token);
        return false;
    }
    return true;
}

bool get_and_expect_token(Serdec *serdec, long token)
{
    stb_c_lexer_get_token(&serdec->l);
    return expect_token(serdec, token);
}

bool get_and_expect_id(Serdec *serdec, const char *name)
{
    if (!get_and_expect_token(serdec, CLEX_id)) return false;
    if (strcmp(serdec->l.string, name) != 0) {
        stb_lex_location loc = {0};
        stb_c_lexer_get_location(&serdec->l, serdec->l.where_firstchar, &loc);
        fprintf(stderr, "%s:%d:%d: ERROR: expected `%s`, but got `%s`\n", serdec->file_path, loc.line_number, loc.line_offset, name, serdec->l.string);
        return false;
    }
    return true;
}

bool parse_type(Serdec *serdec, Type *type)
{
    if (!get_and_expect_token(serdec, CLEX_id)) return false;

    if (strcmp(serdec->l.string, "const") == 0) {
        if (!get_and_expect_id(serdec, "char")) return false;
        if (!get_and_expect_token(serdec, '*')) return false;
        type->kind = TYPE_STRING;
        type->typename = "const char*";
        return true;
    }
    if (strcmp(serdec->l.string, "double") == 0) {
        type->kind = TYPE_DOUBLE;
        type->typename = "double";
        return true;
    }
    if (strcmp(serdec->l.string, "int") == 0) {
        type->kind = TYPE_INT;
        type->typename = "int";
        return true;
    }
    if (strcmp(serdec->l.string, "bool") == 0) {
        type->kind = TYPE_BOOL;
        type->typename = "bool";
        return true;
    }

    if (struct_parsed(serdec, serdec->l.string)) {
        type->kind = TYPE_STRUCT;
        type->typename = strdup(serdec->l.string); // memory leak
        return true;
    }

    static_assert(COUNT_TYPES == 5, "Amount of types have changed");

    stb_lex_location loc = {0};
    stb_c_lexer_get_location(&serdec->l, serdec->l.where_firstchar, &loc);
    fprintf(stderr, "%s:%d:%d: ERROR: unsupported type starting with `%s`\n", serdec->file_path, loc.line_number, loc.line_offset, serdec->l.string);

    return false;
}

bool parse_object(Serdec *serdec, Object *object)
{
    for (;;) {
        char *saved_point = serdec->l.parse_point;
        stb_c_lexer_get_token(&serdec->l);
        if (serdec->l.token == '}') break;
        serdec->l.parse_point = saved_point;

        Field field = {0};
        if (!parse_field(serdec, &field)) return false;
        da_append(object, field);
    }
    return true;
}

bool parse_array(Serdec *serdec, Array *array)
{
    parse_type(serdec, &array->type);
    if (!get_and_expect_token(serdec, '*')) return false;
    if (!get_and_expect_id(serdec, "items")) return false;
    if (!get_and_expect_token(serdec, ';')) return false;
    if (!get_and_expect_id(serdec, "size_t")) return false;
    if (!get_and_expect_id(serdec, "count")) return false;
    if (!get_and_expect_token(serdec, ';')) return false;
    if (!get_and_expect_id(serdec, "size_t")) return false;
    if (!get_and_expect_id(serdec, "capacity")) return false;
    if (!get_and_expect_token(serdec, ';')) return false;
    if (!get_and_expect_token(serdec, '}')) return false;
    return true;
}

bool parse_struct(Serdec *serdec, Struct *strukt)
{
    if (!get_and_expect_id(serdec, "typedef")) return false;
    if (!get_and_expect_id(serdec, "struct"))  return false;
    char *saved_point = serdec->l.parse_point;
    stb_c_lexer_get_token(&serdec->l);
    if (serdec->l.token == CLEX_id && strcmp(serdec->l.string, "array") == 0) {
        if (!get_and_expect_token(serdec, '(')) return false;
        if (!get_and_expect_token(serdec, ')')) return false;
        strukt->kind = STRUCT_ARRAY;
    } else {
        serdec->l.parse_point = saved_point;
    }
    if (!get_and_expect_token(serdec, '{'))    return false;

    switch (strukt->kind) {
    case STRUCT_OBJECT:
        if (!parse_object(serdec, &strukt->as.object)) return false;
        break;
    case STRUCT_ARRAY:
        if (!parse_array(serdec, &strukt->as.array)) return false;
        break;
    case COUNT_STRUCTS: default: UNREACHABLE("strukt->kind");
    }

    if (!get_and_expect_token(serdec, CLEX_id)) return false;
    strukt->name = strdup(serdec->l.string); // memory leak
    if (!get_and_expect_token(serdec, ';'))    return false;
    da_append(&serdec->parsed_structs, strukt->name);
    return true;
}

bool parse_field(Serdec *serdec, Field *field)
{
    if (!parse_type(serdec, &field->type))      return false;
    if (!get_and_expect_token(serdec, CLEX_id)) return false;
    field->name = strdup(serdec->l.string); // memory leak

    char *saved_point = serdec->l.parse_point;
    stb_c_lexer_get_token(&serdec->l);
    if (serdec->l.token == CLEX_id && strcmp(serdec->l.string, "tag") == 0) {
        if (!get_and_expect_token(serdec, '(')) return false;
        if (!get_and_expect_token(serdec, CLEX_dqstring)) return false;
        field->tag = strdup(serdec->l.string); // memory leak
        if (!get_and_expect_token(serdec, ')')) return false;
    } else {
        field->tag = field->name;
        serdec->l.parse_point = saved_point;
    }

    if (!get_and_expect_token(serdec, ';'))     return false;
    return true;
}

bool parse_structs(Serdec *serdec, Structs *structs)
{
    for (;;) {
        char *saved_point = serdec->l.parse_point;
        stb_c_lexer_get_token(&serdec->l);
        if (serdec->l.token == CLEX_eof) return true;
        serdec->l.parse_point = saved_point;

        Struct strukt = {0};
        if (!parse_struct(serdec, &strukt)) return false;
        da_append(structs, strukt);
    }

    UNREACHABLE("parse_structs");
}

#endif // SERDEC_IMPLEMENTATION
