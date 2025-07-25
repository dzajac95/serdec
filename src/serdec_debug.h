// This is a Serdec Generator for debug printing types
#ifndef SERDEC_DEBUG_H_
#define SERDEC_DEBUG_H_

#include "serdec.h"
#define NOB_STRIP_PREFIX
#include "nob.h"

void generate_struct_c_printer(Struct strukt, String_Builder *out);

#endif // SERDEC_DEBUG_H_

#ifdef SERDEC_DEBUG_IMPLEMENTATION

void generate_struct_c_printer(Struct strukt, String_Builder *out)
{
    sb_appendf(out, "void print_%s(%s s)\n", strukt.name, strukt.name);
    sb_appendf(out, "{\n");
    switch (strukt.kind) {
    case STRUCT_OBJECT:
        sb_appendf(out, "    printf(\"(%s) {\\n\");\n", strukt.name);
        da_foreach(Field, field, &strukt.as.object) {
            sb_appendf(out, "    printf(\"    .%s = \");\n", field->name);
            switch (field->type.kind) {
            case TYPE_STRING:
                sb_appendf(out, "    printf(\"\\\"%%s\\\",\\n\", s.%s);\n", field->name);
                break;
            case TYPE_DOUBLE:
                sb_appendf(out, "    printf(\"%%lf,\\n\", s.%s);\n", field->name);
                break;
            case TYPE_INT:
                sb_appendf(out, "    printf(\"%%d,\\n\", s.%s);\n", field->name);
                break;
            case TYPE_BOOL:
                sb_appendf(out, "    printf(\"%%d,\\n\", s.%s);\n", field->name);
                break;
            case TYPE_STRUCT:
                sb_appendf(out, "    print_%s(s.%s);\n", field->type.typename, field->name);
                break;
            case COUNT_TYPES: default: UNREACHABLE("field->type");
            }
        }
        sb_appendf(out, "    printf(\"}\\n\");\n");
        break;
    case STRUCT_ARRAY:
        sb_appendf(out, "    UNUSED(s);\n");
        sb_appendf(out, "    printf(\"(%s): Array of type %s\\n\");\n", strukt.name, strukt.as.array.type.typename);
        break;
    case COUNT_STRUCTS: default: UNREACHABLE("strukt.kind");
    }
    sb_appendf(out, "}\n");
}

#endif // SERDEC_DEBUG_IMPLEMENTATION
