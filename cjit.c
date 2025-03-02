#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "defc/defc.h"
#include "src_lexer/lexer.h"
#include "hashmap/hashmap.h"
#include "parser/parser.h"
#include "misc/file.h"
#include <wchar.h>
#include <sys/resource.h>
#include <stdint.h>
#include <locale.h>

// TODO: add support for several statements, they should be separated by ";"
// TODO: add blocks and function declaration support
// TODO: check error when expression parser put parenthesis to get_token_precedence, so actually just add parenthesis support to parser

// TODO: everytime when there is error in token like expected one but get another, it should enter panic mode and go to next statement after ";" or ")", "}"
// TODO: check for amount of brackets in expression, right now this work fine "(1 + 2) * (3.43 / 54"
// TODO: after function declaration add support for function call
// TODO: after functions and variables start making Syntax checker, that will just check types and if it declared
// TODO: add more complex types like long, unsigned
// TODO: continue adding cyrylic support for lexer(remaka advance, error, checking for character in is alpha)
// TODO: remove error when it fault if last character is space
// TODO: refactor parser into different files
// TODO: add globals to parser
// TODO: add proper error handling
// TODO: add proper function to generate errors with mismatching tokens
// TODO: not free ast when it dont need to
// TODO: one day remake parser so it will be more efficient

typedef struct
{
    char *error;
} CompilingError;

void *convert_string_to_number(const char *str, const TokenType type)
{
    void *result = NULL;
    if (type == T_DNUMBER)
    {
        int *int_value = (int *)malloc(sizeof(int));
        if (int_value != NULL)
        {
            sscanf(str, "%d", int_value);
        }
        result = int_value;
    }
    else if (type == T_FNUMBER)
    {
        float *float_value = (float *)malloc(sizeof(float));
        if (float_value != NULL)
        {
            sscanf(str, "%f", float_value);
        }
        result = float_value;
    }
    return result;
}

parser_literal parse_var_type()
{
    // TODO: add more complex types like long, unsigned
    Token token = get_parser_token(current_parser);
    parser_literal type = {NULL, LITERAL_INT, LITERAL_SIGNED};

    switch (token.type)
    {
    case T_INT:
        type.literal_type = LITERAL_INT;
        break;
    case T_FLOAT:
        type.literal_type = LITERAL_FLOAT;
        break;
    default:
        current_parser->error_found = true;
        // TODO: add proper error handling
        wprintf(L"Unknown token type in parse_var_type: %s\n", token_to_string(token.type));
    }

    advance_parser(current_parser);
    return type;
}

int parse_assignment_expression()
{
    if (peek_parser_token_type(current_parser, T_IDENTIFIER, 0) && peek_parser_token_type(current_parser, T_ASSIGN, 1))
    {
        char *name = get_parser_token(current_parser).value;
        advance_parser(current_parser);

        consume_parser_token(current_parser, T_ASSIGN);

        int expression = parse_assignment_expression();
        return add_ast_node(current_parser, cast_assignment_node(name, expression));
    }

    return parse_expression(current_parser, 0);
}

ASTNode cast_declaration_node(char *name, parser_literal var_type, int expression)
{
    ASTNode node;
    node.type = N_VARIABLE_DECLARATION;
    node.data.variable_declaration.name = name;
    node.data.variable_declaration.literal = var_type;
    node.data.variable_declaration.expression = expression;
    return node;
}

void add_variable_declaration(parser_literal var_type)
{
    parser_declaration *declaration = malloc(sizeof(parser_declaration));
    declaration->type = VARIABLE_DECLARATION;
    declaration->literal = var_type;

    hashmap_insert(current_parser->declarations, var_type.value, declaration);
}

int parse_declaration(Parser *parser)
{
    parser_literal var_type = parse_var_type();
    int decl_node;

    while (true)
    {
        Token token = consume_parser_token(current_parser, T_IDENTIFIER);
        var_type.value = token.value;

        add_variable_declaration(var_type);
        consume_parser_token(current_parser, T_ASSIGN);

        int value = parse_assignment_expression();

        decl_node = add_ast_node(current_parser, cast_declaration_node(token.value, var_type, value));

        // TODO: remake declarations parcing so it could handle chain of declarations "int x = 5, y = 10, z = 15;"
        break;
        // if(!peek_parser_token_type(current_parser, T_COMMA, 0)){
        //     break;
        // }
        // advance_parser(current_parser);
    }

    consume_parser_token(current_parser, T_SEMICOLON);
    return decl_node;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        wprintf(L"Usage: %s <file>\n", argv[0]);
        return 1;
    }
    setlocale(LC_CTYPE, "en_US.UTF-8");

    int file_size;
    char *source = read_file(argv[1], &file_size);

    HashMap *lexers_hashmap = init_hashmap();
    Lexer *lexer = lex_source(source, strdup(argv[1]));
    hashmap_insert(lexers_hashmap, strdup(argv[1]), lexer);
    current_lexer = hashmap_get(lexers_hashmap, argv[1]);

    // print_tokens(current_lexer);

    Parser *parser = init_parser(current_lexer);
    current_parser = parser;

    // int head = parse_expression(parser, 0);
    int head = parse_declaration(parser);
    print_ast_node(parser, head, 0);

    // for (int i = 0; i < parser->ast_count; i++)
    // {
    //     wprintf(L"AST node %d type: %d\n", i, parser->ast_nodes[i].type);
    // }

    free_parser(parser, true);
    free_hashmap(lexers_hashmap, free_lexer_wrapper);

    return 0;
}