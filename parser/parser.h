#include "../src_lexer/lexer.h"
#include "../hashmap/hashmap.h"

#ifndef PARSER_H
#define PARSER_H

typedef enum
{
    LITERAL_CHAR,
    LITERAL_SHORT,
    LITERAL_INT,
    LITERAL_LONG,
    LITERAL_FLOAT,
    LITERAL_DOUBLE,
    LITERAL_LDOUBLE,
} LiteralType;

typedef enum
{
    LITERAL_UNSIGNED,
    LITERAL_SIGNED
} LiteralSign;

typedef enum
{
    FUNCTION_DECLARATION,
    VARIABLE_DECLARATION
} DeclarationType;

typedef enum
{
    N_BINARY_EXPRESSION,
    N_UNARY_EXPRESSION,
    N_LITERAL,
    N_VARIABLE_DECLARATION,
    N_ASSIGNMENT,
} NodeType;

typedef struct
{
    char *value;
    LiteralType literal_type;
    LiteralSign literal_sign;
} parser_literal;

typedef struct
{
    DeclarationType type;
    union
    {
        parser_literal literal;
    };
} parser_declaration;

typedef struct
{
    NodeType type;

    union
    {

        parser_literal literal;

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

        struct
        {
            char *name;
            int expression;
        } assignment;

        struct
        {
            char *name;
            parser_literal literal;
            int expression;
        } variable_declaration;
    } data;
} ASTNode;

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

    HashMap *symbol_table;
    HashMap *declarations;

} Parser;

// parse structures
int primary(Parser *parser);
int parse_expression(Parser *parser, int precedence);
int parse_unary_expression(Parser *parser);

// parser struct
Parser *init_parser(Lexer *lexer);
void free_parser(Parser *parser, bool free_tokens);
void free_declaration(parser_declaration *declaration);
int advance_parser(Parser *parser);
int get_token_precedence(TokenType type);
Token get_parser_token(Parser *parser);
int add_ast_node(Parser *parser, ASTNode node);
Token consume_parser_token(Parser *parser, TokenType expected_type);
int match_parser_token_type(Parser *parser, TokenType expected_type, int offset);
Token peek_parser_token(Parser *parser, int offset);
int peek_parser_token_type(Parser *parser, TokenType expected_type, int offset);
void free_parser_declaration(parser_declaration *declaration);
void free_parser_declaration_wrapper(void *value);
void resize_ast_array(Parser *parser);

// casting
ASTNode cast_binary_node(TokenType type, int left, int right);
ASTNode cast_unary_node(TokenType type, int expression);
ASTNode cast_literal_node(LiteralType type, void *value);
ASTNode cast_assignment_node(char *name, int expression);

// utils
void print_ast_indent(int indent_level);
void print_ast_node(Parser *parser, int node_index, int indent_level);

#endif