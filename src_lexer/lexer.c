#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <stdbool.h>
#include "../defc/defc.h"
#include "../misc/string.h"

Lexer *init_lexer(char *source, char *file_name)
{
    Lexer *lexer = malloc(sizeof(Lexer));
    lexer->source = source;

    lexer->file_name = file_name;

    lexer->tokens = malloc(TOKEN_INCREMENT * sizeof(Token));
    lexer->token_capacity = TOKEN_INCREMENT;
    lexer->token_count = 0;

    lexer->line = 1;
    lexer->column = 1;
    lexer->position = 0;
    lexer->length = strlen(source);
    lexer->current_char = source[0];
    lexer->is_eof = false;

    return lexer;
}

void free_lexer(Lexer *lexer, bool free_tokens)
{
    if (free_tokens)
    {
        for (int i = 0; i < lexer->token_count; i++)
        {
            free(lexer->tokens[i].value);
        }
        free(lexer->tokens);
    }
    free(lexer->source);
    free(lexer->file_name);
    free(lexer);
}

void free_token(Token *token)
{
    free(token->value);
    free(token);
}

void free_lexer_wrapper(void *value)
{
    free_lexer((Lexer *)value, false);
}

void push_token(Lexer *lexer, Token *token)
{
    if (lexer->token_count >= lexer->token_capacity - 1)
    {
        lexer->token_capacity += TOKEN_INCREMENT;
        lexer->tokens = realloc(lexer->tokens, lexer->token_capacity * sizeof(Token));
        if (lexer->tokens == NULL)
        {
            wprintf(L"Error reallocating memory\n");
            exit(1);
        }
    }

    lexer->tokens[lexer->token_count] = *token;
    lexer->token_count++;
}

Lexer *lex_source(char *source, char *file_name)
{
    Lexer *lexer = init_lexer(source, file_name);

    while (lexer->is_eof == false)
    {
        Token *token = get_next_token(lexer);
        push_token(lexer, token);
    }

    char *eof = (char *)malloc(2);
    eof[0] = 'f';
    eof[1] = '\0';
    Token *eof_token = init_token(T_EOF, eof, lexer->line, lexer->column, lexer->position, lexer->position);
    push_token(lexer, eof_token);

    return lexer;
}

char peek_next_char(Lexer *lexer)
{
    if (lexer->position + 1 <= lexer->length && lexer->source[lexer->position + 1] != '\0')
    {
        return lexer->source[lexer->position + 1];
    }
    lexer->is_eof = true;
    return '\0';
}

int get_utf8_char_len(unsigned char first_byte)
{
    if ((first_byte & 0x80) == 0)
    {
        return 1;
    }
    else if ((first_byte & 0xE0) == 0xC0)
    {
        return 2;
    }
    else if ((first_byte & 0xF0) == 0xE0)
    {
        return 3;
    }
    else if ((first_byte & 0xF8) == 0xF0)
    {
        return 4;
    }
    else
    {
        return 0;
    }
}

// void advance_lexer(Lexer *lexer)
// {
//     if (lexer->current_char != '\0')
//     {
//         unsigned char current_byte = lexer->current_char;
//         if (current_byte >= 0x80)
//         {
//             int char_len = get_utf8_char_len(current_byte);

//             if (lexer->position + char_len > lexer->length)
//             {
//                 lexer->is_eof = true;
//             }
//             lexer->position += char_len;
//             lexer->column += char_len;
//             lexer->current_char = lexer->source[lexer->position];
//         }
//         else
//         {
//             lexer->position++;
//             lexer->column++;
//             lexer->current_char = lexer->source[lexer->position];
//         }
//     }
//     else
//     {
//         lexer->is_eof = true;
//     }
// }

void advance_lexer(Lexer *lexer)
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
        lexer->is_eof = true;
    }
}

Token *init_token(TokenType type, char *value, int line, int column, int position, int end_position)
{
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->value = value;
    token->line = line;
    token->column = column;
    token->position = position;
    token->end_position = end_position;
    return token;
}

int isalpha_cyrillic(int ch)
{
    if ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))
    {
        return 1;
    }
    if ((ch >= 0x0410 && ch <= 0x042F) || (ch >= 0x0430 && ch <= 0x044F) || ch == 0x0404 || ch == 0x0454 || ch == 0x0406 || ch == 0x0456 || ch == 0x0407 || ch == 0x0457)
    {
        return 1;
    }
    return 0;
}

Token *get_next_token(Lexer *lexer)
{
    while (lexer->is_eof == false)
    {
        skip_whitespace(lexer);

        if (isdigit(lexer->current_char))
        {
            return get_number_token(lexer);
        }

        if (isalpha_cyrillic(lexer->current_char) || lexer->current_char == '_')
        {
            return get_identifier_token(lexer);
        }

        char *value = (char *)malloc(2);
        value[0] = lexer->current_char;
        value[1] = '\0';
        Token *token = init_token(T_UNKNOWN, value, lexer->line, lexer->column, lexer->position, 0);

        switch (lexer->current_char)
        {
        case '/':
            token->type = T_DIVIDE;
            break;
        case '(':
            token->type = T_LPAREN;
            break;
        case ')':
            token->type = T_RPAREN;
            break;
        case ';':
            token->type = T_SEMICOLON;
            break;
        case '=':
            if (peek_next_char(lexer) == '=')
            {
                advance_lexer(lexer);
                token->type = T_EQUAL;
                append_token_value(token, "=");
            }
            else
            {
                token->type = T_ASSIGN;
            }
            break;
        case '+':
            if (peek_next_char(lexer) == '+')
            {
                advance_lexer(lexer);
                token->type = T_INCREMENT;
                append_token_value(token, "+");
            }
            else
            {
                token->type = T_PLUS;
            }
            break;
        case '-':
            if (peek_next_char(lexer) == '-')
            {
                advance_lexer(lexer);
                token->type = T_DECREMENT;
                append_token_value(token, "-");
            }
            else
            {
                token->type = T_MINUS;
            }
            break;
        case '*':
            if (peek_next_char(lexer) == '*')
            {
                advance_lexer(lexer);
                token->type = T_POWER;
                append_token_value(token, "*");
            }
            else
            {
                token->type = T_MULTIPLY;
            }
            break;
        default:
            char *line = lexer_get_line(lexer);
            char *wrapped_line = wrap_text_part(line, lexer->column - 1, lexer->column + strlen(token->value) - 2, "\033[1;4;31m", "\033[0m");
            // TODO: remake this code for cyrilic
            wprintf(L"%s:%d:%d ERROR: unknown token: %c\n", lexer->file_name, lexer->line, lexer->column, lexer->current_char);
            // printf("%s:%d:%d ERROR: unknown token: \033[1;35m`%c`\033[0m\n\033[1m%d\033[0m | %s\n",
            //        lexer->file_name, lexer->line, lexer->column, lexer->current_char, lexer->line, wrapped_line);
            free(line);
            free(token->value);
            free(token);
            free(wrapped_line);
            advance_lexer(lexer);
            continue;
        }

        advance_lexer(lexer);
        token->end_position = lexer->position;
        return token;
    }

    Token *token = init_token(T_EOF, "", lexer->line, lexer->column, lexer->position, strlen(lexer->source));
    return token;
}

TokenType get_token_keyword(char *value)
{
    if (strcmp(value, "int") == 0)
    {
        return T_INT;
    }
    if (strcmp(value, "float") == 0)
    {
        return T_FLOAT;
    }
    else if (strcmp(value, "else") == 0)
    {
        return T_ELSE;
    }
    else if (strcmp(value, "while") == 0)
    {
        return T_WHILE;
    }
    else if (strcmp(value, "for") == 0)
    {
        return T_FOR;
    }
    else if (strcmp(value, "return") == 0)
    {
        return T_RETURN;
    }
    return T_IDENTIFIER;
}

Token *get_identifier_token(Lexer *lexer)
{
    char *buffer = malloc(256 * sizeof(char));
    int i = 0;

    while (lexer->current_char != '\0' && (isalnum(lexer->current_char) || lexer->current_char == '_' || isalpha_cyrillic(lexer->current_char)))
    {
        buffer[i++] = lexer->current_char;
        advance_lexer(lexer);
    }

    buffer = realloc(buffer, (i + 1) * sizeof(char));
    buffer[i] = '\0';

    TokenType type = get_token_keyword(buffer);
    Token *token = init_token(type, buffer, lexer->line, lexer->column - i, lexer->position - i, lexer->position - 1);

    return token;
}

Token *get_number_token(Lexer *lexer)
{
    char *buffer = malloc(256 * sizeof(char));
    int i = 0;
    bool is_float = false;

    while (lexer->current_char != '\0' && isdigit(lexer->current_char))
    {
        buffer[i++] = lexer->current_char;
        advance_lexer(lexer);
    }

    if (lexer->current_char == '.')
    {
        is_float = true;
        buffer[i++] = lexer->current_char;
        advance_lexer(lexer);

        while (lexer->current_char != '\0' && isdigit(lexer->current_char))
        {
            buffer[i++] = lexer->current_char;
            advance_lexer(lexer);
        }
    }

    buffer = realloc(buffer, (i + 1) * sizeof(char));
    buffer[i] = '\0';

    Token *token = init_token(is_float ? T_FNUMBER : T_DNUMBER, buffer, lexer->line, lexer->column - i, lexer->position - i, lexer->position);

    return token;
}

void append_token_value(Token *token, char *value)
{
    token->value = realloc(token->value, strlen(token->value) + strlen(value) + 1);
    strcat(token->value, value);
}

char *lexer_get_line(Lexer *lexer)
{
    int start_pos = lexer->position;
    int end_pos = lexer->position;

    while (start_pos > 0 && lexer->source[start_pos - 1] != '\n')
    {
        start_pos--;
    }

    while (lexer->source[end_pos - 1] == '\n' || lexer->source[end_pos] != '\0')
    {
        end_pos++;
    }

    char *line = (char *)malloc(end_pos - start_pos + 1);
    strncpy(line, &lexer->source[start_pos], end_pos - start_pos);
    line[end_pos - start_pos] = '\0';

    return line;
}

void skip_whitespace(Lexer *lexer)
{
    while (lexer->is_eof == false && isspace(lexer->current_char))
    {
        if (lexer->current_char == '\n')
        {
            lexer->line++;
            lexer->column = 1;
        }
        advance_lexer(lexer);
    }
}

const char *token_to_string(TokenType type)
{
    switch (type)
    {
    case T_EOF:
        return "EOF";
    case T_IDENTIFIER:
        return "Identifier";
    case T_DNUMBER:
        return "Decimal Number";
    case T_FNUMBER:
        return "Floating Number";
    case T_STRING:
        return "String";
    case T_INT:
        return "Integer Keyword";
    case T_FLOAT:
        return "Float Keyword";
    case T_BOOL:
        return "Boolean Keyword";
    case T_IF:
        return "If Keyword";
    case T_ELSE:
        return "Else Keyword";
    case T_FOR:
        return "For Keyword";
    case T_WHILE:
        return "While Keyword";
    case T_RETURN:
        return "Return Keyword";
    case T_PLUS:
        return "Plus Operator";
    case T_MINUS:
        return "Minus Operator";
    case T_LPAREN:
        return "Left Parenthesis";
    case T_RPAREN:
        return "Right Parenthesis";
    case T_INCREMENT:
        return "Increment Operator";
    case T_DECREMENT:
        return "Decrement Operator";
    case T_MULTIPLY:
        return "Multiply Operator";
    case T_POWER:
        return "Power Operator";
    case T_DIVIDE:
        return "Divide Operator";
    case T_ASSIGN:
        return "Assignment Operator";
    case T_EQUAL:
        return "Equal Operator";
    case T_NOT_EQUAL:
        return "Not Equal Operator";
    case T_GREATER:
        return "Greater Operator";
    case T_GREATER_EQUAL:
        return "Greater or Equal Operator";
    case T_LESS:
        return "Less Operator";
    case T_SEMICOLON:
        return "Semicolon";
    default:
        return "Unknown Token Type in token_to_string()";
    }
}

void print_tokens(Lexer *lexer)
{
    for (int i = 0; i < lexer->token_count; i++)
    {
        wprintf(L"%s:%d:%d \033[1;35m%s[%d]\033[0m: %s\n", lexer->file_name, lexer->tokens[i].line, lexer->tokens[i].column, token_to_string(lexer->tokens[i].type), lexer->tokens[i].end_position - lexer->tokens[i].position, lexer->tokens[i].value);
    }
}
