#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "lexer/lexer.h"

char *read_file(const char *filename, int *file_size)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    char *source = (char *)malloc(*file_size + 1);
    if (source == NULL)
    {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    fread(source, *file_size, 1, file);
    source[*file_size] = '\0';

    fclose(file);
    return source;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int file_size;
    char *source = read_file(argv[1], &file_size);
    if (source == NULL)
    {
        printf("Failed to read file: %s\n", argv[1]);
    }

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