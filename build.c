#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char *argv[])
{

    const char *command = "gcc cjit.c lexer/lexer.c -o cjit; ./cjit example1.cj";
    int result = system(command);

    return 0;
}