#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
    const char *command = "gcc -g -o cjit cjit.c lexer/lexer.c; ./cjit example1.cj";
    int result = system(command);

    if (result == -1)
    {
        fprintf(stderr, "Error executing command: %s\n", strerror(errno));
        return 1;
    }

    return 0;
}
