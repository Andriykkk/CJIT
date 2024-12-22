#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "defc/defc.h"
#include "src_lexer/lexer.h"
#include "hashmap/hashmap.h"

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

// void free_lexer_wrapper(void *value)
// {
//     free_lexer((Lexer *)value, true);
// }

Lexer *current_lexer = NULL;

// TODO: add parser to parse expression
// TODO: check hashmap freeing memory
// TODO: add iterator to hashmap

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

    HashMap *lexers_hashmap = init_hashmap();

    Lexer *lexer = lex_source(source, strdup(argv[1]));

    hashmap_insert(lexers_hashmap, strdup(argv[1]), lexer);

    current_lexer = hashmap_find(lexers_hashmap, argv[1]);
    print_tokens(current_lexer);

    // free_hashmap(lexers_hashmap, free_lexer_wrapper);

    return 0;
}