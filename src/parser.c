#include <stdio.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#include "nob.h"
#define JIM_IMPLEMENTATION
#include "jim.h"
#define JIMP_IMPLEMENTATION
#include "jimp.h"

#define tag(...)

#include "person.h"
#include "serdec_person.h"

int main(int argc, char **argv)
{
    const char *program_name = shift(argv, argc);

    if (argc <= 0) {
        fprintf(stderr, "Usage: %s <input>\n", program_name);
        fprintf(stderr, "ERROR: no input is provided\n");
        return 1;
    }
    const char *file_path = shift(argv, argc);

    Person person = {0};
    String_Builder sb = {0};
    Jimp jimp = {0};
    Jim jim = {.pp = 4};

    if (!read_entire_file(file_path, &sb)) return 1;

    jimp_begin(&jimp, file_path, sb.items, sb.count);
    if (!jimp_Person(&jimp, &person)) return 1;

    printf("Parsed C struct from %s:\n", file_path);
    print_Person(person);
    printf("\n");
    printf("Serialized back as JSON:\n");
    jim_Person(&jim, person);
    fwrite(jim.sink, jim.sink_count, 1, stdout);
    printf("\n");

    return 0;
}
