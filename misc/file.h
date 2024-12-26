#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

char *read_file(const char *filename, int *file_size)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        wprintf(L"Error opening file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    char *source = (char *)malloc(*file_size + 1);
    if (source == NULL)
    {
        wprintf(L"Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    fread(source, *file_size, 1, file);
    source[*file_size] = '\0';

    fclose(file);
    return source;
}