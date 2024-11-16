#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int get_file_mtime(const char *path, time_t *mtime)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        return -1;
    }
    *mtime = st.st_mtime;
    return 0;
}

int compile_is_needed(const char *file_path)
{
    char compiled_path[1024];
    snprintf(compiled_path, sizeof(compiled_path), "%s.out", file_path);
}

int main(int argc, char *argv[])
{

    // const char *command = "gcc cjit.c -o cjit; ./cjit example1.cj";
    const char *main_file = "cjit.c";

    int result = system(command);

    if (!compile_is_needed(main_file))
    {
        return 1;
    }

    // char command[1024];
    // snprintf(run_command, sizeof(run_command), "./%s.out", source_path);

    // if (result == -1)
    // {
    //     perror("system");
    //     return 1;
    // }

    // printf("Command executed with exit status: %d\n", result);
    return 0;
}