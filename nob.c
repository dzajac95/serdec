#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#if defined(__GNUC__)
#  define NOB_REBUILD_URSELF(binary_path, source_path) "cc", "-Wall", "-Wextra", "-Wswitch-enum", "-ggdb", "-o", binary_path, source_path
#endif
#define nob_cc_include_path(cmd, path) cmd_append(cmd, temp_sprintf("-I%s", path))
#include "./thirdparty/nob.h"

#define BUILD_FOLDER "./build/"
#define SRC_FOLDER "./src/"
#define THIRDPARTY_FOLDER "./thirdparty/"

bool build_executable(Cmd *cmd, const char *src_path, const char *bin_path)
{
    nob_cc(cmd);
    nob_cc_flags(cmd);
    nob_cc_include_path(cmd, THIRDPARTY_FOLDER);
    nob_cc_include_path(cmd, SRC_FOLDER);
    nob_cc_include_path(cmd, BUILD_FOLDER);
    nob_cc_inputs(cmd, src_path);
    nob_cc_output(cmd, bin_path);
    return cmd_run_sync_and_reset(cmd);
}

int main(int argc, char **argv)
{
    NOB_GO_REBUILD_URSELF(argc, argv);

    if (!mkdir_if_not_exists(BUILD_FOLDER)) return 1;

    Cmd cmd = {0};

    if (!build_executable(&cmd, SRC_FOLDER"generator.c", BUILD_FOLDER"generator")) return 1;
    cmd_append(&cmd, BUILD_FOLDER"generator", SRC_FOLDER"person.h", BUILD_FOLDER"serdec_person.h");
    if (!cmd_run_sync_and_reset(&cmd)) return 1;
    if (!build_executable(&cmd, SRC_FOLDER"parser.c", BUILD_FOLDER"parser")) return 1;

    return 0;
}
