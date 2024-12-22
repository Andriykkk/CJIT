#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>
#include "lexer/lexer.h"

#define MAX_ERRORS 255
jmp_buf EOF_EXIT;

char *read_file(const char *filename, int *file_size)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        printf("Error opening file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    rewind(file);

    char *source = (char *)malloc(*file_size + 1);
    if (source == NULL)
    {
        printf("Error allocating memory\n");
        fclose(file);
        return NULL;
    }

    fread(source, *file_size, 1, file);
    source[*file_size] = '\0';

    fclose(file);
    return source;
}

typedef enum
{
    NODE_ROOT,
    NODE_FUNCTION,
    NODE_BLOCK,
    NODE_VARIABLE_DECLARATION,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_BINARY_EXPRESSION,
    NODE_UNARY_EXPRESSION,
    NODE_IDENTIFIER,
    NODE_INUMBER,
    NODE_FNUMBER,
    NODE_CALL_EXPRESSION,
    NODE_ASSIGNMENT
} NodeType;

typedef enum
{
    SYNTAX_ERROR,
    SEMANTIC_ERROR
} CompilingError;

typedef struct ASTNode
{
    NodeType type;

    union
    {
        int int_value; // numbers
        float float_value;

        char *string_value; // strings

        // binary expression
        struct
        {
            struct ASTNode *left;
            struct ASTNode *right;
            TokenType operator;
        } binary;

        struct
        {
            TokenType operator;
            struct ASTNode *expression;
        } unary;

        struct
        {
            struct ASTNode *expression;
            struct ASTNode *then_block;
            struct ASTNode *else_block;
        } if_statements;

        struct
        {
            char *name;
            struct ASTNode *arguments;
            int parameter_count;
            struct ASTNode *body;
            char *return_type;
        } function;

        struct
        {
            struct ASTNode *left;
            struct ASTNode *right;
        } assignment;

        struct
        {
            char *name;
            char *type;
            struct ASTNode *expression;
        } variable_declarations;

        struct
        {
            struct ASTNode *expression;
        } block;
    } data;
} ASTNode;

typedef struct
{
    CompilingError type;
    char *message;
    int line;
    int column;
} ParseError;

typedef struct
{
    Lexer *lexer;
    Token *current_token;
    Token *previous_token;
    ParseError *errors;
    int error_count;
} Parser;

ASTNode *parse(Parser *parser);
ASTNode *parse_primary(Parser *parser);
ASTNode *parse_binary_expression(Parser *parser);
ASTNode *parse_statements(Parser *parser);

char *node_type_to_string(NodeType type)
{
    switch (type)
    {
    case NODE_ROOT:
        return "Root";
    case NODE_FUNCTION:
        return "Function";
    case NODE_BLOCK:
        return "Block";
    case NODE_VARIABLE_DECLARATION:
        return "Variable Declaration";
    case NODE_IF_STATEMENT:
        return "If Statement";
    case NODE_WHILE_STATEMENT:
        return "While Statement";
    case NODE_RETURN_STATEMENT:
        return "Return Statement";
    case NODE_BINARY_EXPRESSION:
        return "Binary Expression";
    case NODE_UNARY_EXPRESSION:
        return "Unary Expression";
    case NODE_IDENTIFIER:
        return "Identifier";
    case NODE_INUMBER:
        return "Integer Number";
    case NODE_FNUMBER:
        return "Float Number";
    case NODE_CALL_EXPRESSION:
        return "Call Expression";
    case NODE_ASSIGNMENT:
        return "Assignment";
    default:
        return "Unknown";
    };
}

Parser *init_parser(Lexer *lexer)
{
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->current_token = get_next_token(lexer);
    parser->previous_token = NULL;
    parser->errors = malloc(sizeof(ParseError) * 255);
    parser->error_count = 0;

    return parser;
}

int advance_parser(Parser *parser)
{
    parser->previous_token = parser->current_token;
    parser->current_token = get_next_token(parser->lexer);
    if (parser->current_token == NULL)
        return 0;

    return 1;
}

void print_ast(ASTNode *node, int level)
{
    for (int i = 0; i < level; i++)
    {
        printf("\t");
    }
    if (node == NULL)
    {
        printf("NULL\n");
        return;
    }
    switch (node->type)
    {
    case NODE_ROOT:
        printf("Root:\n");
        print_ast(node->data.block.expression, level + 1);
        break;
    case NODE_ASSIGNMENT:
        printf("Assignment:\n");
        print_ast(node->data.assignment.left, level + 1);

        for (int i = 0; i < level + 1; i++)
        {
            printf("\t");
        }

        print_ast(node->data.assignment.right, level + 1);
        break;
    case NODE_VARIABLE_DECLARATION:
        printf("Variable Declaration: %s %s\n", node->data.variable_declarations.type, node->data.variable_declarations.name);

        print_ast(node->data.variable_declarations.expression, level + 1);
        break;
    case NODE_CALL_EXPRESSION:
        printf("Call Expression:\n");
        print_ast(node->data.block.expression, level + 1);
        break;
    case NODE_INUMBER:
        printf("Integer: %d\n", node->data.int_value);
        break;
    case NODE_FNUMBER:
        printf("Float: %f\n", node->data.float_value);
        break;
    case NODE_BINARY_EXPRESSION:
        printf("Binary Expression:\n");
        print_ast(node->data.binary.left, level + 1);

        for (int i = 0; i < level + 1; i++)
        {
            printf("\t");
        }

        printf("%s\n", token_type_to_string(node->data.binary.operator));

        print_ast(node->data.binary.right, level + 1);
        break;
    case NODE_UNARY_EXPRESSION:
        printf("Unary Expression:\n");

        for (int i = 0; i < level + 1; i++)
        {
            printf("\t");
        }
        printf("%s\n", token_type_to_string(node->data.unary.operator));
        print_ast(node->data.unary.expression, level + 1);
        break;
    case NODE_IDENTIFIER:
        printf("Identifier: %s\n", node->data.string_value);
        break;
    default:
        printf("Unknown node type %s \n", node_type_to_string(node->type));
        break;
    }
}

int match(Parser *parser, TokenType type)
{
    if (parser->current_token->type == type)
    {
        advance_parser(parser);
        return 1;
    }
    return 0;
}

void print_errors(Parser *parser)
{
    for (int i = 0; i < parser->error_count; i++)
    {
        printf("%s", parser->errors[i].message);
    }
}

void record_error(Parser *parser, char *message, CompilingError error_type)
{
    char error_message[1024];
    switch (error_type)
    {
    case SYNTAX_ERROR:
        snprintf(error_message, 1024, "[Syntax Error] Line %d, Column %d: %s\n", parser->previous_token->line, parser->previous_token->column, message);
        break;
    case SEMANTIC_ERROR:
        snprintf(error_message, 1024, "[Semantic Error] Line %d, Column %d: %s\n", parser->previous_token->line, parser->previous_token->column, message);
        break;
    default:
        snprintf(error_message, 1024, "[Unknown Error] Line %d, Column %d: %s\n", parser->previous_token->line, parser->previous_token->column, message);
        break;
    }

    if (parser->error_count < MAX_ERRORS - 1)
    {
        parser->errors[parser->error_count].type = error_type;
        parser->errors[parser->error_count].message = error_message;
        parser->errors[parser->error_count].line = parser->previous_token->line;
        parser->errors[parser->error_count].column = parser->previous_token->column;
        parser->error_count++;
    }
    else
    {
        print_errors(parser);
        printf("Too many errors\n");
        exit(1);
    }
}

void skip_to_next_valid_token(Parser *parser)
{
    while (parser->current_token != NULL)
    {
        if (parser->current_token->type == TOKEN_SEMICOLON ||
            parser->current_token->type == TOKEN_RIGHT_BRACE ||
            parser->current_token->type == TOKEN_RIGHT_PAREN)
        {
            return;
        }

        advance_parser(parser);
    }
}

void skip_and_record_error(Parser *parser, char *message, CompilingError error_type)
{
    record_error(parser, message, error_type);
    skip_to_next_valid_token(parser);
}

int advance_parser_exit(Parser *parser)
{
    parser->previous_token = parser->current_token;
    parser->current_token = get_next_token(parser->lexer);
    if (parser->current_token == NULL)
    {
        char error_message[1024];
        snprintf(error_message, 1024, "Expected expression, got EOF");
        skip_and_record_error(parser, error_message, SYNTAX_ERROR);
        longjmp(EOF_EXIT, 1);
    }
}

int expect(Parser *parser, TokenType type)
{
    if (parser->current_token->type == type)
    {
        advance_parser(parser);
        return 0;
    }
    else
    {
        char error_message[1024];
        snprintf(error_message, 1024, "Expected %s, got %s", token_type_to_string(type), token_type_to_string(parser->current_token->type));
        record_error(parser, error_message, SYNTAX_ERROR);
        return -1;
    }
}

ASTNode *parse(Parser *parser)
{
    ASTNode *root = (ASTNode *)malloc(sizeof(ASTNode));

    root = parse_statements(parser);
    return root;
}

ASTNode *parse_statements(Parser *parser)
{
    ASTNode *node = parse_primary(parser);

    if (node == NULL)
    {
        return NULL;
    }
    printf("node type: %d\n", parser->current_token->type);
    // if (expect(parser, TOKEN_SEMICOLON))
    // {
    //     return NULL;
    // }

    return node;
}

ASTNode *parse_primary(Parser *parser)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));

    if (parser->current_token->type == TOKEN_FLOAT || parser->current_token->type == TOKEN_INT)
    {
        char *type = strdup(parser->current_token->value);
        advance_parser_exit(parser);

        if (parser->current_token->type == TOKEN_IDENTIFIER)
        {
            char *name = strdup(parser->current_token->value);
            advance_parser(parser);

            if (expect(parser, TOKEN_ASIGN))
            {
                return NULL;
            }

            ASTNode *expression = parse_binary_expression(parser);

            node->type = NODE_VARIABLE_DECLARATION;
            node->data.variable_declarations.type = type;
            node->data.variable_declarations.name = name;
            node->data.variable_declarations.expression = expression;
            return node;
        }
        else
        {
            char error_message[1024];
            snprintf(error_message, 1024, "Expected identifier, got %s", token_type_to_string(parser->current_token->type));
            skip_and_record_error(parser, error_message, SYNTAX_ERROR);
            return NULL;
        }
    }

    if (parser->current_token->type == TOKEN_INUMBER)
    {

        node->type = NODE_INUMBER;
        node->data.int_value = atoi(parser->current_token->value);
        advance_parser(parser);
        return node;
    }

    if (parser->current_token->type == TOKEN_FNUMBER)
    {
        node->type = NODE_FNUMBER;
        node->data.float_value = atof(parser->current_token->value);
        advance_parser(parser);

        return node;
    }

    // prefix increment and decrement
    if (parser->current_token->type == TOKEN_INCREMENT || parser->current_token->type == TOKEN_DECREMENT)
    {
        TokenType type = parser->current_token->type;
        advance_parser_exit(parser);

        char *name = strdup(parser->current_token->value);

        node->type = NODE_UNARY_EXPRESSION;
        node->data.unary.operator= type;
        node->data.unary.expression = parse_primary(parser);
        return node;
    }

    if (parser->current_token->type == TOKEN_IDENTIFIER)
    {
        char *name = strdup(parser->current_token->value);
        advance_parser(parser);

        if (parser->current_token != NULL && parser->current_token->type == TOKEN_LEFT_PAREN)
        {
            node->type = NODE_CALL_EXPRESSION;

            return node;
        }

        node->type = NODE_IDENTIFIER;
        node->data.string_value = name;
        return node;
    }

    if (parser->current_token->type == TOKEN_LEFT_PAREN)
    {
        advance_parser(parser);
        ASTNode *expression = parse_binary_expression(parser);
        expect(parser, TOKEN_RIGHT_PAREN);
        return expression;
    }

    return NULL;
}

ASTNode *parse_binary_expression(Parser *parser)
{
    ASTNode *left = parse_primary(parser);

    if (left == NULL)
    {
        return NULL;
    }

    while (parser->current_token != NULL && (parser->current_token->type == TOKEN_PLUS ||
                                             parser->current_token->type == TOKEN_MINUS ||
                                             parser->current_token->type == TOKEN_MULTIPLY ||
                                             parser->current_token->type == TOKEN_DIVIDE))
    {
        TokenType operator= parser->current_token->type;

        advance_parser(parser);
        if (parser->current_token == NULL)
        {
            char error_message[1024];
            snprintf(error_message, 1024, "Expected expression, got EOF");
            skip_and_record_error(parser, error_message, SYNTAX_ERROR);
            return left;
        }

        ASTNode *right = parse_primary(parser);

        ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
        node->type = NODE_BINARY_EXPRESSION;
        node->data.binary.left = left;
        node->data.binary.right = right;
        node->data.binary.operator= operator;

        left = node;
    }

    return left;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int file_size;
    char *source = read_file(argv[1], &file_size);
    if (source == NULL)
    {
        printf("Failed to read file: %s\n", argv[1]);
    }

    Lexer *lexer = init_lexer(source);
    Parser *parser = init_parser(lexer);

    ASTNode *root = (ASTNode *)malloc(sizeof(ASTNode));
    if (setjmp(EOF_EXIT) == 0)
    {
        root = parse(parser);
    }

    print_ast(root, 0);
    print_errors(parser);

    return 0;
}