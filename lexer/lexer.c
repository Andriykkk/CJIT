#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

Lexer *init_lexer(char *source)
{
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->position = 0;
    lexer->length = strlen(source);
    lexer->current_char = source[0];
    return lexer;
}

void advance(Lexer *lexer)
{
    lexer->position++;
    lexer->column++;

    if (lexer->position < lexer->length)
    {
        lexer->current_char = lexer->source[lexer->position];
    }
    else
    {
        lexer->current_char = '\0';
    }
}

void skip_whitespace(Lexer *lexer)
{
    while (lexer->current_char != '\0' && isspace(lexer->current_char))
    {
        if (lexer->current_char == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        advance(lexer);
    }
}

char *token_type_to_string(TokenType type)
{
    switch (type)
    {
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_INUMBER:
        return "INUMBER";
    case TOKEN_FNUMBER:
        return "FNUMBER";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_INT:
        return "INT";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_BOOL:
        return "BOOL";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_FOR:
        return "FOR";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_INCREMENT:
        return "INCREMENT";
    case TOKEN_DECREMENT:
        return "DECREMENT";
    case TOKEN_MULTIPLY:
        return "MULTIPLY";
    case TOKEN_POWER:
        return "POWER";
    case TOKEN_DIVIDE:
        return "DIVIDE";
    case TOKEN_ASIGN:
        return "ASIGN";
    case TOKEN_EQUAL:
        return "EQUAL";
    case TOKEN_NOT_EQUAL:
        return "NOT_EQUAL";
    case TOKEN_GREATER:
        return "GREATER";
    case TOKEN_GREATER_EQUAL:
        return "GREATER_EQUAL";
    case TOKEN_LESS:
        return "LESS";
    case TOKEN_LESS_EQUAL:
        return "LESS_EQUAL";
    case TOKEN_LEFT_PAREN:
        return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TOKEN_LEFT_BRACKET:
        return "LEFT_BRACKET";
    default:
        return "UNKNOWN TOKEN IN TOKEN_TYPE_TO_STRING";
    }
}

TokenType get_token_type(char *value)
{
    // Keywords
    if (strcmp(value, "int") == 0)
        return TOKEN_INT;
    if (strcmp(value, "float") == 0)
        return TOKEN_FLOAT;
    if (strcmp(value, "bool") == 0)
        return TOKEN_BOOL;
    if (strcmp(value, "if") == 0)
        return TOKEN_IF;
    if (strcmp(value, "else") == 0)
        return TOKEN_ELSE;
    if (strcmp(value, "for") == 0)
        return TOKEN_FOR;
    if (strcmp(value, "while") == 0)
        return TOKEN_WHILE;
    if (strcmp(value, "return") == 0)
        return TOKEN_RETURN;

    // Preprocessor
    if (strcmp(value, "import") == 0)
        return TOKEN_IMPORT;
    return TOKEN_IDENTIFIER;
}

char peek_next(Lexer *lexer)
{
    if (lexer->position + 1 <= lexer->length && lexer->source[lexer->position + 1] != '\0')
    {
        return lexer->source[lexer->position + 1];
    }
    return '\0';
}

char peek_prev(Lexer *lexer)
{
    if (lexer->position - 1 >= 0 && lexer->source[lexer->position - 1] != '\0')
    {
        return lexer->source[lexer->position - 1];
    }
    return '\0';
}

Token *get_number(Lexer *lexer)
{
    char buffer[256] = {0};
    int i = 0;
    bool is_float = false;

    while (lexer->current_char != '\0' && isdigit(lexer->current_char))
    {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    if (lexer->current_char == '.')
    {
        is_float = true;
        buffer[i++] = lexer->current_char;
        advance(lexer);

        while (lexer->current_char != '\0' && isdigit(lexer->current_char))
        {
            buffer[i++] = lexer->current_char;
            advance(lexer);
        }
    }

    Token *token = (Token *)malloc(sizeof(Token));
    token->value = strdup(buffer);
    if (is_float)
    {
        token->type = TOKEN_FNUMBER;
    }
    else
    {
        token->type = TOKEN_INUMBER;
    }
    token->line = lexer->line;
    token->column = lexer->column;

    return token;
}

Token *get_preprocessor(Lexer *lexer)
{
    char buffer[256] = {0};
    int i = 0;

    if (lexer->current_char == '#' && i == 0)
    {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    while (lexer->current_char != '\0' && (isalnum(lexer->current_char) || lexer->current_char == '_'))
    {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    Token *token = (Token *)malloc(sizeof(Token));
    token->value = strdup(buffer);
    token->type = get_token_type(buffer);
    token->line = lexer->line;
    token->column = lexer->column;

    return token;
}

Token *get_identifier(Lexer *lexer)
{
    char buffer[256] = {0};
    int i = 0;

    while (lexer->current_char != '\0' && (isalnum(lexer->current_char) || lexer->current_char == '_'))
    {
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    Token *token = (Token *)malloc(sizeof(Token));
    token->value = strdup(buffer);
    token->type = get_token_type(buffer);
    token->line = lexer->line;
    token->column = lexer->column;

    return token;
}

Token *get_string(Lexer *lexer)
{
    int max_length = 256;
    char *buffer = (char *)malloc(max_length * sizeof(char));
    char string_type = 0;
    bool escape_char = false;
    int i = 0;

    if (lexer->current_char == '"' || lexer->current_char == '\'')
    {
        string_type = lexer->current_char;
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    while (lexer->current_char != '\0')
    {
        if (i >= max_length - 1 && max_length < 2147483647 / 2)
        {
            max_length *= 2;
            buffer = (char *)realloc(buffer, max_length * sizeof(char));
        }

        if (lexer->current_char == '\\' && !escape_char)
        {
            escape_char = true;
            buffer[i++] = lexer->current_char;
            advance(lexer);
            continue;
        }

        if (lexer->current_char == string_type && !escape_char)
        {
            buffer[i++] = lexer->current_char;
            advance(lexer);
            break;
        }

        buffer[i++] = lexer->current_char;
        escape_char = false;
        advance(lexer);
    }

    buffer[i] = '\0';

    Token *token = (Token *)malloc(sizeof(Token));

    token->value = (char *)malloc(i + 1);
    strcpy(token->value, buffer);

    token->type = TOKEN_STRING;
    token->line = lexer->line;
    token->column = lexer->column;

    free(buffer);

    return token;
}

Token *get_one_line_comment(Lexer *lexer)
{
    int max_length = 256;
    char *buffer = (char *)malloc(max_length * sizeof(char));
    int i = 0;

    buffer[i++] = lexer->current_char;
    advance(lexer);
    buffer[i++] = lexer->current_char;
    advance(lexer);

    while (lexer->current_char != '\0' && peek_next(lexer) != '\n')
    {
        if (i >= max_length - 1 && max_length < 2147483647 / 2)
        {
            max_length *= 2;
            buffer = (char *)realloc(buffer, max_length * sizeof(char));
        }
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    buffer[i] = '\0';

    Token *token = (Token *)malloc(sizeof(Token));

    token->value = (char *)malloc(i + 1);
    strcpy(token->value, buffer);

    token->type = TOKEN_STRING;
    token->line = lexer->line;
    token->column = lexer->column;

    free(buffer);

    return token;
}

Token *get_multi_line_comment(Lexer *lexer)
{
    int max_length = 256;
    char *buffer = (char *)malloc(max_length * sizeof(char));
    int i = 0;

    buffer[i++] = lexer->current_char;
    advance(lexer);
    buffer[i++] = lexer->current_char;
    advance(lexer);

    while (lexer->current_char != '\0' && !(lexer->current_char == '*' && peek_next(lexer) == '/'))
    {
        if (i >= max_length - 1 && max_length < 2147483647 / 2)
        {
            max_length *= 2;
            buffer = (char *)realloc(buffer, max_length * sizeof(char));
        }
        buffer[i++] = lexer->current_char;
        advance(lexer);
    }

    buffer[i++] = lexer->current_char;
    advance(lexer);
    buffer[i++] = lexer->current_char;
    advance(lexer);

    buffer[i] = '\0';

    Token *token = (Token *)malloc(sizeof(Token));

    token->value = (char *)malloc(i + 1);
    strcpy(token->value, buffer);

    token->type = TOKEN_STRING;
    token->line = lexer->line;
    token->column = lexer->column;

    free(buffer);

    return token;
}

void append_token_value(Token *token, char *value)
{
    token->value = realloc(token->value, strlen(token->value) + strlen(value) + 1);
    strcat(token->value, value);
}

Token *get_next_token(Lexer *lexer)
{
    while (lexer->current_char != '\0')
    {
        if (isspace(lexer->current_char))
        {
            skip_whitespace(lexer);
            continue;
        }

        if (lexer->current_char == '#')
        {
            return get_preprocessor(lexer);
        }

        if (lexer->current_char == '"' || lexer->current_char == '\'')
        {
            return get_string(lexer);
        }

        if (lexer->current_char == '/' && peek_next(lexer) == '/')
        {
            return get_one_line_comment(lexer);
        }

        if (lexer->current_char == '/' && peek_next(lexer) == '*')
        {
            return get_multi_line_comment(lexer);
        }

        if (isalpha(lexer->current_char) || lexer->current_char == '_')
        {
            return get_identifier(lexer);
        }

        if (isdigit(lexer->current_char))
        {
            return get_number(lexer);
        }

        Token *token = (Token *)malloc(sizeof(Token));
        token->line = lexer->line;
        token->column = lexer->column;
        token->value = (char *)malloc(2);
        token->value[0] = lexer->current_char;
        token->value[1] = '\0';

        switch (lexer->current_char)
        {
        case ';':
            token->type = TOKEN_SEMICOLON;
            break;
        case '(':
            token->type = TOKEN_LEFT_PAREN;
            break;
        case ')':
            token->type = TOKEN_RIGHT_PAREN;
            break;
        case '{':
            token->type = TOKEN_LEFT_BRACE;
            break;
        case '}':
            token->type = TOKEN_RIGHT_BRACE;
            break;
        case ',':
            token->type = TOKEN_COMMA;
            break;
        case '.':
            token->type = TOKEN_DOT;
            break;
        case '/':
            token->type = TOKEN_DIVIDE;
            break;
        case '[':
            token->type = TOKEN_LEFT_BRACKET;
            break;
        case ']':
            token->type = TOKEN_RIGHT_BRACKET;
            break;
        case '+':
            if (peek_next(lexer) == '+')
            {
                advance(lexer);
                token->type = TOKEN_INCREMENT;
                append_token_value(token, "+");
            }
            else
            {
                token->type = TOKEN_PLUS;
            }
            break;
        case '-':
            if (peek_next(lexer) == '-')
            {
                advance(lexer);
                token->type = TOKEN_DECREMENT;
                append_token_value(token, "-");
            }
            else
            {
                token->type = TOKEN_MINUS;
            }
            break;
        case '*':
            if (peek_next(lexer) == '*')
            {
                advance(lexer);
                token->type = TOKEN_POWER;
                append_token_value(token, "*");
            }
            else
            {
                token->type = TOKEN_MULTIPLY;
            }
            break;
        case '!':
            if (peek_next(lexer) == '=')
            {
                advance(lexer);
                token->type = TOKEN_NOT_EQUAL;
                append_token_value(token, "=");
            }
            else
            {
                token->type = TOKEN_NOT;
            }
            break;
        case '>':
            if (peek_next(lexer) == '=')
            {
                advance(lexer);
                token->type = TOKEN_GREATER_EQUAL;
                append_token_value(token, "=");
            }
            else
            {
                token->type = TOKEN_GREATER;
            }
            break;
        case '<':
            if (peek_next(lexer) == '=')
            {
                advance(lexer);
                token->type = TOKEN_LESS_EQUAL;
                append_token_value(token, "=");
            }
            else
            {
                token->type = TOKEN_LESS;
            }
            break;
        case '=':
            if (peek_next(lexer) == '=')
            {
                advance(lexer);
                token->type = TOKEN_EQUAL;
                append_token_value(token, "=");
            }
            else
            {
                token->type = TOKEN_ASIGN;
            }
            break;
        case '&':
            if (peek_next(lexer) == '&')
            {
                advance(lexer);
                token->type = TOKEN_AND;
                append_token_value(token, "&");
            }
            else
            {
                token->type = TOKEN_NOT_IMPLEMENTED;
            }
            break;
        case '|':
            if (peek_next(lexer) == '|')
            {
                advance(lexer);
                token->type = TOKEN_OR;
                append_token_value(token, "|");
            }
            else
            {
                token->type = TOKEN_NOT_IMPLEMENTED;
            }
            break;
        default:
            free(token->value);
            free(token);
            printf("ERROR: Unknown token: %c\n at line %d, column %d", lexer->current_char, lexer->line, lexer->column);
            advance(lexer);
            continue;
        }

        advance(lexer);
        return token;
    }

    return NULL;
}
