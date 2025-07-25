// This is a Serdec Generator for jim.h/jimp.h immediate serialization/deserialization libraries.
#ifndef SERDEC_JIM_H_
#define SERDEC_JIM_H_

#include "serdec.h"
#define NOB_STRIP_PREFIX
#include "nob.h"

void generate_struct_jim_serializer(Struct strukt, String_Builder *out);
void generate_struct_jimp_deserializer(Struct strukt, String_Builder *out);

#endif // SERDEC_JIM_H_

#ifdef SERDEC_JIM_IMPLEMENTATION

void generate_struct_jim_serializer(Struct strukt, String_Builder *out)
{
    sb_appendf(out, "void jim_%s(Jim *jim, %s s)\n", strukt.name, strukt.name);
    sb_appendf(out, "{\n");
    if (strukt.kind == STRUCT_OBJECT) {
        sb_appendf(out, "    jim_object_begin(jim);\n");
        da_foreach(Field, field, &strukt.as.object) {
            sb_appendf(out, "        jim_member_key(jim, \"%s\");\n", field->tag);
            switch (field->type.kind) {
            case TYPE_STRING:
                sb_appendf(out, "        jim_string(jim, s.%s);\n", field->name);
                break;
            case TYPE_DOUBLE:
                sb_appendf(out, "        jim_float(jim, s.%s, 4);\n", field->name);
                break;
            case TYPE_INT:
                sb_appendf(out, "        jim_integer(jim, s.%s);\n", field->name);
                break;
            case TYPE_BOOL:
                sb_appendf(out, "        jim_bool(jim, s.%s);\n", field->name);
                break;
            case TYPE_STRUCT:
                sb_appendf(out, "        jim_%s(jim, s.%s);\n", field->type.typename, field->name);
                break;
            case COUNT_TYPES: default: UNREACHABLE("field->type");
            }
        }
        sb_appendf(out, "    jim_object_end(jim);\n");
    } else if (strukt.kind == STRUCT_ARRAY) {
        sb_appendf(out, "    jim_array_begin(jim);\n");
        Type array_type = strukt.as.array.type;
        sb_appendf(out, "    da_foreach(%s, it, &s) {\n", array_type.typename);
        switch (array_type.kind) {
            case TYPE_STRING:
                sb_appendf(out, "        jim_string(jim, *it);\n");
                break;
            case TYPE_DOUBLE:
                sb_appendf(out, "        jim_float(jim, *it, 4);\n");
                break;
            case TYPE_INT:
                sb_appendf(out, "        jim_integer(jim, *it);\n");
                break;
            case TYPE_BOOL:
                sb_appendf(out, "        jim_bool(jim, *it);\n");
                break;
            case TYPE_STRUCT:
                sb_appendf(out, "        jim_%s(jim, *it);\n", array_type.typename);
                break;
            case COUNT_TYPES: default: UNREACHABLE("field->type");
        }
        sb_appendf(out, "    }\n");
        sb_appendf(out, "    jim_array_end(jim);\n");
    }
    sb_appendf(out, "}\n");
}

void jimp_object_deserializer(Object object, String_Builder *out)
{
    sb_appendf(out, "    if (!jimp_object_begin(jimp)) return false;\n");
    sb_appendf(out, "    while (jimp_object_member(jimp)) {\n");
    da_foreach(Field, field, &object) {
        sb_appendf(out, "        if (strcmp(jimp->string, \"%s\") == 0) {\n", field->tag); // TODO: tag should be properly escaped
        switch (field->type.kind) {
        case TYPE_STRING:
            sb_appendf(out, "            if (!jimp_string(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = strdup(jimp->string); // memory leak\n", field->name);
            break;
        case TYPE_DOUBLE:
            sb_appendf(out, "            if (!jimp_number(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = jimp->number;\n", field->name);
            break;
        case TYPE_INT:
            sb_appendf(out, "            if (!jimp_number(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = jimp->number;\n", field->name);
            break;
        case TYPE_BOOL:
            sb_appendf(out, "            if (!jimp_bool(jimp)) return false;\n");
            sb_appendf(out, "            s->%s = jimp->boolean;\n", field->name);
            break;
        case TYPE_STRUCT:
            sb_appendf(out, "            if (!jimp_%s(jimp, &s->%s)) return false;\n", field->type.typename, field->name);
            break;
        case COUNT_TYPES: default: UNREACHABLE("field->type");
        }
        sb_appendf(out, "            continue;\n");
        sb_appendf(out, "        }\n");
    }
    sb_appendf(out, "    }\n");
    sb_appendf(out, "    if (!jimp_object_end(jimp)) return false;\n");
    sb_appendf(out, "    return true;\n");
}

void jimp_array_deserializer(Array array, String_Builder *out)
{
    sb_appendf(out, "    if (!jimp_array_begin(jimp)) return false;\n");
    sb_appendf(out, "    while (jimp_array_item(jimp)) {\n");
    switch (array.type.kind) {
    case TYPE_STRING:
        sb_appendf(out, "        if (!jimp_string(jimp)) return false;\n");
        sb_appendf(out, "        da_append(s, strdup(jimp->string))); // memory leak\n");
        break;
    case TYPE_DOUBLE:
        sb_appendf(out, "        if (!jimp_number(jimp)) return false;\n");
        sb_appendf(out, "        da_append(s, jimp->number);\n");
        break;
    case TYPE_INT:
        sb_appendf(out, "        if (!jimp_number(jimp)) return false;\n");
        sb_appendf(out, "        da_append(s, jimp->number);\n");
        break;
    case TYPE_BOOL:
        sb_appendf(out, "        if (!jimp_bool(jimp)) return false;\n");
        sb_appendf(out, "        da_append(s, jimp->boolean);\n");
        break;
    case TYPE_STRUCT:
        sb_appendf(out, "        %s item = {0};\n", array.type.typename);
        sb_appendf(out, "        if (!jimp_%s(jimp, &item)) return false;\n", array.type.typename);
        sb_appendf(out, "        da_append(s, item);\n");
        break;
    case COUNT_TYPES: default: UNREACHABLE("array.type.kind");
    }
    sb_appendf(out, "    }\n");
    sb_appendf(out, "    if (!jimp_array_end(jimp)) return false;\n");
    sb_appendf(out, "    return true;\n");
}

void generate_struct_jimp_deserializer(Struct strukt, String_Builder *out)
{
    sb_appendf(out, "bool jimp_%s(Jimp *jimp, %s *s)\n", strukt.name, strukt.name);
    sb_appendf(out, "{\n");
    switch(strukt.kind) {
    case STRUCT_OBJECT:
        jimp_object_deserializer(strukt.as.object, out);
        break;
    case STRUCT_ARRAY:
        jimp_array_deserializer(strukt.as.array, out);
        break;
    case COUNT_STRUCTS: default: UNREACHABLE("strukt.kind");
    }
    sb_appendf(out, "}\n");
}

#endif // SERDEC_JIM_IMPLEMENTATION
