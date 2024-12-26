#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "defc/defc.h"
#include "src_lexer/lexer.h"
#include "hashmap/hashmap.h"
#include "misc/file.h"
#include <wchar.h>
#include <sys/resource.h>
#include <stdint.h>
#include <locale.h>

// TODO: think how to create globals and local variables
// TODO: continue adding cyrylic support for lexer(remaka advance, error, checking for character in is alpha)
// TODO: remove error when it fault if last character is space
// TODO: refactor parser into different files
// TODO: add globals to parser
// TODO: add proper function to generate errors with mismatching tokens
// TODO: not free ast when it dont need to
// TODO: one day remake parser so it will be more efficient

Lexer *current_lexer = NULL;

typedef enum
{
    LITERAL_INT,
    LITERAL_LONG,
    LITERAL_UNSIGNED,
    LITERAL_SIGNED,
    LITERAL_FLOAT,
    LITERAL_DOUBLE,
    LITERAL_STRING
} LiteralType;

typedef enum
{
    N_BINARY_EXPRESSION,
    N_UNARY_EXPRESSION,
    N_LITERAL,
} NodeType;

typedef struct
{
    NodeType type;

    union
    {
        struct
        {
            char *value;
            LiteralType literal_type;
        } literal;

        struct
        {
            int left;
            int right;
            TokenType operator;
        } binary;

        struct
        {
            TokenType operator;
            int expression;
        } unary;
    } data;
} ASTNode;

typedef struct
{
    char *error;
} CompilingError;

typedef struct
{
    Lexer *lexer;
    Token *tokens;
    char *file_name;
    int token_count;
    int current_token_index;
    bool error_found;
    bool is_eol;

    ASTNode *ast_nodes;
    int ast_size;
    int ast_count;
} Parser;

Parser *current_parser = NULL;

Parser *init_parser(Lexer *lexer)
{
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->tokens = lexer->tokens;
    parser->file_name = lexer->file_name;
    parser->token_count = lexer->token_count;
    parser->current_token_index = 0;
    parser->is_eol = false;
    parser->error_found = false;

    parser->ast_size = PARSER_INCREMENT;
    parser->ast_count = 0;

    parser->ast_nodes = malloc(parser->ast_size * sizeof(ASTNode));
    if (!parser->ast_nodes)
    {
        fprintf(stderr, "Memory allocation failed for AST nodes.\n");
        exit(1);
    }

    return parser;
}

typedef union
{
    int int_value;
    float float_value;
} Number;

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
        return 0;
    }
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
    free(parser->ast_nodes);
    free(parser);
}

void advance_parser(Parser *parser)
{
    if (parser->current_token_index < parser->token_count)
    {
        parser->current_token_index++;
    }
    parser->is_eol = true;
}

Token get_parser_token(Parser *parser)
{
    return parser->tokens[parser->current_token_index];
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
        printf("Expected token of type %d, but got token of type %d\n", expected_type, current_token.type);
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
        parser->error_found = true;
        // TODO: add error
    }
    return 0;
}

int cast_binary_node(TokenType type, int left, int right)
{
    ASTNode node;

    node.type = N_BINARY_EXPRESSION;
    node.data.binary.left = left;
    node.data.binary.right = right;
    node.data.binary.operator= type;

    return add_ast_node(current_parser, node);
}

ASTNode cast_unary_node(TokenType type, int expression)
{
    ASTNode node;

    node.type = N_UNARY_EXPRESSION;
    node.data.unary.expression = expression;
    node.data.unary.operator= type;

    return node;
}

ASTNode cast_literal_node(LiteralType type, void *value)
{
    ASTNode node;
    node.type = N_LITERAL;
    node.data.literal.literal_type = type;
    node.data.literal.value = value;

    return node;
}

int primary(Parser *parser);
int parse_expression(Parser *parser, int precedence);

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
        printf("Unknown token type: %d\n", token.type);
        break;
    }

    advance_parser(parser);
    return node;
}

int parse_expression(Parser *parser, int precedence)
{
    int left = parse_unary_expression(parser);

    while (true)
    {
        Token token = get_parser_token(parser);
        int token_precedence = get_token_precedence(token.type);

        if (token_precedence <= precedence)
        {
            break;
        }

        consume_parser_token(parser, token.type);

        int right = parse_expression(parser, token_precedence);

        int node = cast_binary_node(token.type, left, right);

        left = node;
    }

    return left;
}

void print_ast_indent(int indent_level)
{
    for (int i = 0; i < indent_level; i++)
    {
        printf("\t");
    }
}

void print_ast_node(Parser *parser, int node_index, int indent_level)
{
    if (node_index >= parser->ast_count)
        return;

    ASTNode node = parser->ast_nodes[node_index];
    print_ast_indent(indent_level);

    switch (node.type)
    {
    case N_LITERAL:
    {
        printf("Literal: ");
        if (node.data.literal.literal_type == LITERAL_FLOAT)
            printf("Float: %s\n", node.data.literal.value);
        else if (node.data.literal.literal_type == LITERAL_INT)
            printf("Int: %s\n", node.data.literal.value);
        else
            printf("Unknown Literal Type\n");
    }
    break;
    case N_UNARY_EXPRESSION:
    {
        printf("Unary: ");
        switch (node.data.unary.operator)
        {
        case T_PLUS:
            printf("+\n");
            break;
        case T_MINUS:
            printf("-\n");
            break;
        default:
            printf("Unknown Unary Operator\n");
        }
        print_ast_node(parser, node.data.unary.expression, indent_level + 1);
    }
    break;

    case N_BINARY_EXPRESSION:
    {
        printf("Binary: ");
        switch (node.data.binary.operator)
        {
        case T_PLUS:
            printf("+\n");
            break;
        case T_MINUS:
            printf("-\n");
            break;
        case T_MULTIPLY:
            printf("*\n");
            break;
        case T_DIVIDE:
            printf("/\n");
            break;
        default:
            printf("Unknown Binary Operator\n");
        }
        print_ast_indent(indent_level);
        printf("Left operand:\n");
        print_ast_node(parser, node.data.binary.left, indent_level + 1);

        print_ast_indent(indent_level);
        printf("Right operand:\n");
        print_ast_node(parser, node.data.binary.right, indent_level + 1);
    }
    break;

    default:
        printf("Unknown node type\n");
    }
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
    int head = parse_expression(parser, 0);
    print_ast_node(parser, head, 0);

    // for (int i = 0; i < parser->ast_count; i++)
    // {
    //     printf("AST node %d:\n", i);
    // }

    free_parser(parser, true);
    free_hashmap(lexers_hashmap, free_lexer_wrapper);

    return 0;
}