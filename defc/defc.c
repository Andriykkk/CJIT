#include "../src_lexer/lexer.h"
#include "../parser/parser.h"
#include <stddef.h>

Lexer *current_lexer = NULL;
Parser *current_parser = NULL;