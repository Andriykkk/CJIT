#include <stdbool.h>

#ifndef LEXER_H
#define LEXER_H

typedef enum
{
    T_EOF = 0,
    T_IDENTIFIER,
    T_DNUMBER,
    T_FNUMBER,
    T_STRING,

    // Keywords
    T_INT,
    T_FLOAT,
    T_BOOL,
    T_IF,
    T_ELSE,
    T_FOR,
    T_WHILE,
    T_RETURN,

    // Operators
    T_PLUS,
    T_MINUS,
    T_INCREMENT,
    T_DECREMENT,
    T_MULTIPLY,
    T_POWER,
    T_DIVIDE,
    T_ASSIGN,
    T_EQUAL,
    T_NOT_EQUAL,
    T_GREATER,
    T_GREATER_EQUAL,
    T_LESS,
    T_LESS_EQUAL,
    T_AND,
    T_OR,
    T_NOT,
    T_BAND,
    T_BOR,
    T_BNOT,

    // Syntax
    T_SEMICOLON,
    T_LPAREN,
    T_RPAREN,
    T_LBRACE,
    T_RBRACE,
    T_LBRACKET,
    T_RBRACKET,
    T_COMMA,
    T_DOT,
    T_DOUBLE_QUOTE,
    T_SINGLE_QUOTE,
    T_COMMENT,

    // Preprocessor
    T_IMPORT,
    // TOKEN_DEFINE,
    // TOKEN_UNDEF,
    // TOKEN_IFDEF,
    // TOKEN_IFNDEF,
    // TOKEN_ELIF,
    // TOKEN_ENDIF,
    // TOKEN_INCLUDE,

    T_NOT_IMPLEMENTED,
    T_UNKNOWN

} TokenType;

typedef struct
{
    TokenType type;
    char *value;
    int line;
    int column;
    int position;
    int end_position;
} Token;

typedef struct
{
    char *source;

    Token *tokens;
    int token_capacity;
    int token_count;

    char *file_name;

    int position;
    int length;
    int line;
    int column;
    char current_char;
    bool is_eof;
} Lexer;

Lexer *lex_source(char *source, char *file_name);
void print_tokens(Lexer *lexer);
const char *token_to_string(TokenType type);
Lexer *init_lexer(char *source, char *file_name);
void free_lexer(Lexer *lexer, bool free_tokens);
void free_token(Token *token);
void push_token(Lexer *lexer, Token *token);
char peek_next_char(Lexer *lexer);
void advance_lexer(Lexer *lexer);

Token *init_token(TokenType type, char *value, int line, int column, int position, int end_position);
Token *get_next_token(Lexer *lexer);
Token *get_identifier_token(Lexer *lexer);
Token *get_number_token(Lexer *lexer);
void append_token_value(Token *token, char *value);
char *lexer_get_line(Lexer *lexer);
void skip_whitespace(Lexer *lexer);
void free_lexer_wrapper(void *value);

int isalpha_cyrillic(int ch);

#endif