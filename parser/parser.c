#include "parser.h"
#include "../src_lexer/lexer.h"
#include "../hashmap/hashmap.h"
#include "../defc/defc.h"
#include "casting.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

Token get_parser_token(Parser *parser)
{
    return parser->tokens[parser->current_token_index];
}

int primary(Parser *parser)
{
    Token token = get_parser_token(parser);
    int node;

    switch (token.type)
    {
    case T_DNUMBER:
        node = add_ast_node(parser, cast_literal_node(LITERAL_INT, token.value));
        break;
    case T_FNUMBER:
        node = add_ast_node(parser, cast_literal_node(LITERAL_FLOAT, token.value));
        break;
    case T_LPAREN:
        consume_parser_token(parser, T_LPAREN);
        node = parse_expression(parser, 0);
        match_parser_token_type(parser, T_RPAREN, 0);
        break;

    default:
        parser->error_found = true;
        // TODO: add error
        wprintf(L"Unknown token type in primary: %s\n", token_to_string(token.type));
        exit(1);
        break;
    }

    advance_parser(parser);
    return node;
}

Token peek_parser_token(Parser *parser, int offset)
{
    if (parser->current_token_index + offset >= parser->token_count || parser->current_token_index + offset < 0)
    {
        parser->is_eol = true;
        return parser->tokens[parser->token_count - 1];
    }
    return parser->tokens[parser->current_token_index + offset];
}

int peek_parser_token_type(Parser *parser, TokenType expected_type, int offset)
{
    return peek_parser_token(parser, offset).type == expected_type;
}

int match_parser_token_type(Parser *parser, TokenType expected_type, int offset)
{
    Token token = peek_parser_token(parser, offset);
    if (token.type == expected_type)
    {
        return 1;
    }
    else
    {
        wprintf(L"Expected %s but got %s\n", token_to_string(expected_type), token_to_string(token.type));
        parser->error_found = true;

        // TODO: add error
        exit(1);
    }
    return 0;
}

int parse_expression(Parser *parser, int precedence)
{
    int left = parse_unary_expression(parser);

    while (true)
    {
        Token token = get_parser_token(parser);
        int token_precedence = get_token_precedence(token.type);

        if (token_precedence <= precedence || current_parser->is_eol)
        {
            break;
        }

        consume_parser_token(parser, token.type);
        int right = parse_expression(parser, token_precedence);

        int node = add_ast_node(parser, cast_binary_node(token.type, left, right));

        left = node;
    }

    return left;
}

Parser *init_parser(Lexer *lexer)
{
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->tokens = lexer->tokens;
    parser->file_name = lexer->file_name;
    parser->token_count = lexer->token_count - 1;
    parser->current_token_index = 0;
    parser->is_eol = false;
    parser->error_found = false;

    parser->ast_size = PARSER_INCREMENT;
    parser->ast_count = 0;

    parser->symbol_table = init_hashmap();

    parser->declarations = init_hashmap();

    parser->ast_nodes = malloc(parser->ast_size * sizeof(ASTNode));
    if (!parser->ast_nodes)
    {
        wprintf(L"Memory allocation failed while initializing AST nodes.\n");
        exit(1);
    }

    return parser;
}

void free_parser(Parser *parser, bool free_tokens)
{
    if (free_tokens)
    {
        for (int i = 0; i < parser->token_count; i++)
        {
            free(parser->tokens[i].value);
        }
        free(parser->tokens);
    }
    free_hashmap(parser->symbol_table, free);
    free_hashmap(parser->declarations, free_parser_declaration_wrapper);
    free(parser->ast_nodes);
    free(parser);
}

void free_parser_declaration(parser_declaration *declaration)
{
    if (declaration->type == VARIABLE_DECLARATION)
    {
        free(declaration->literal.value);
    }

    free(declaration);
}

void free_parser_declaration_wrapper(void *value)
{
    free_parser_declaration((parser_declaration *)value);
}

void resize_ast_array(Parser *parser)
{
    int new_size = parser->ast_size + PARSER_INCREMENT;
    parser->ast_nodes = realloc(parser->ast_nodes, new_size * sizeof(ASTNode));
    if (!parser->ast_nodes)
    {
        fprintf(stderr, "Memory allocation failed while resizing AST nodes.\n");
        exit(1);
    }
    parser->ast_size = new_size;
}

int advance_parser(Parser *parser)
{
    if (parser->current_token_index < parser->token_count - 1)
    {
        parser->current_token_index++;
        return 0;
    }
    parser->is_eol = true;
    return 1;
}

Token consume_parser_token(Parser *parser, TokenType expected_type)
{
    Token current_token = get_parser_token(parser);
    if (current_token.type == expected_type)
    {
        advance_parser(parser);
        return current_token;
    }
    else
    {
        // TODO: make proper error handling
        wprintf(L"Expected %s but got %s\n", token_to_string(expected_type), token_to_string(current_token.type));
        exit(1);
        // char *line = lexer_get_line(parser->lexer);
        // char *wrapped_line = wrap_text_part(line, current_token.column - 1, current_token.column + strlen(current_token.value) - 2, "\033[1;4;31m", "\033[0m");
        // printf("%s:%d:%d ERROR: unknown token: \033[1;35m`%c`\033[0m\n\033[1m%d\033[0m | %s\n",
        //        parser->file_name, current_token.line, current_token.column, lexer->current_char, lexer->line, wrapped_line);
        // free(line);
        // free(token->value);
        // free(token);
        // free(wrapped_line);
        // advance_lexer(lexer);
    }
}

int get_token_precedence(TokenType type)
{
    switch (type)
    {
    case T_PLUS:
    case T_MINUS:
        return 1;
    case T_MULTIPLY:
    case T_DIVIDE:
        return 2;
    default:
        wprintf(L"Unknown token type in get_token_precedence: %s\n", token_to_string(type));
        return 0;
    }
}

int add_ast_node(Parser *parser, ASTNode node)
{
    if (parser->ast_count + 1 >= parser->ast_size)
    {
        resize_ast_array(parser);
    }

    int index = parser->ast_count;
    parser->ast_nodes[index] = node;
    parser->ast_count++;

    return index;
}

int parse_unary_expression(Parser *parser)
{
    Token token = get_parser_token(parser);
    if (peek_parser_token_type(parser, T_PLUS, 0) || peek_parser_token_type(parser, T_MINUS, 0))
    {
        advance_parser(parser);
        int operand = primary(parser);
        return operand;
    }

    return primary(parser);
}