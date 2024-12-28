#ifndef DEFC_H
#define DEFC_H

#include "../src_lexer/lexer.h"
#include "../parser/parser.h"
#include <stddef.h>

extern Lexer *current_lexer;
extern Parser *current_parser;

#define MAX_ERRORS 255

#define TOKEN_INCREMENT 1024

#define PARSER_INCREMENT 1024

#define HASHMAP_SIZE 1024

#endif