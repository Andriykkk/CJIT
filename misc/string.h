#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *wrap_text_part(const char *line, int start, int end, const char *start_tag, const char *end_tag)
{
    int line_len = strlen(line);

    if (start < 0 || end > line_len || start > end)
    {
        fprintf(stderr, "Invalid start or end position.\n");
        return NULL;
    }

    int before_len = start;
    int wrapped_len = end - start + 1;
    int after_len = line_len - (end + 1);

    int total_len = before_len + strlen(start_tag) + wrapped_len + strlen(end_tag) + after_len + 1;

    char *wrapped_text = (char *)malloc(total_len);
    if (wrapped_text == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    strncpy(wrapped_text, line, before_len);

    strcpy(wrapped_text + before_len, start_tag);

    strncpy(wrapped_text + before_len + strlen(start_tag), line + start, wrapped_len);

    strcpy(wrapped_text + before_len + strlen(start_tag) + wrapped_len, end_tag);

    strcpy(wrapped_text + before_len + strlen(start_tag) + wrapped_len + strlen(end_tag), line + end + 1);

    return wrapped_text;
}