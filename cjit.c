#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer/lexer.h"

int main(int argc, char *argv[])
{
    FILE *file;
    char *source;
    int file_size;

    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        printf("Error opening file: %s\n", argv[1]);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    source = (char *)malloc(file_size + 1);
    if (source == NULL)
    {
        printf("Error allocating memory\n");
        fclose(file);
        return 1;
    }

    fread(source, file_size, 1, file);
    source[file_size] = '\0';

    fclose(file);

    Lexer *lexer = init_lexer(source);
    Token *token;

    int i = 0;
    while (lexer->position < lexer->length)
    {
        // if (i++ > 15)
        //     break;
        token = get_next_token(lexer);
        if (token != NULL)
            printf("Token: %s, Position: %d, Line: %d, Column: %d\n", token->value, token->type, token->line, token->column);
    }

    return 0;
}