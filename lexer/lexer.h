typedef enum
{
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_INUMBER,
    TOKEN_FNUMBER,
    TOKEN_STRING,

    // Keywords
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_BOOL,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_RETURN,

    // Operators
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_POWER,
    TOKEN_DIVIDE,
    TOKEN_ASIGN,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,

    // Syntax
    TOKEN_SEMICOLON,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_DOUBLE_QUOTE,
    TOKEN_SINGLE_QUOTE,
    TOKEN_COMMENT,

    // Preprocessor
    TOKEN_IMPORT,
    // TOKEN_DEFINE,
    // TOKEN_UNDEF,
    // TOKEN_IFDEF,
    // TOKEN_IFNDEF,
    // TOKEN_ELIF,
    // TOKEN_ENDIF,
    // TOKEN_INCLUDE,

    TOKEN_NOT_IMPLEMENTED

} TokenType;

typedef struct
{
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct
{
    char *source;
    int position;
    int length;
    int line;
    int column;
    char current_char;
} Lexer;

Lexer *init_lexer(char *source);

Token *get_next_token(Lexer *lexer);