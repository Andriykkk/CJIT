#include "parser.h"

#ifndef PARSER_CASTING
#define PARSER_CASTING

ASTNode cast_binary_node(TokenType type, int left, int right)
{
    ASTNode node;

    node.type = N_BINARY_EXPRESSION;
    node.data.binary.left = left;
    node.data.binary.right = right;
    node.data.binary.operator= type;

    return node;
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

ASTNode cast_assignment_node(char *name, int expression)
{
    ASTNode node;
    node.type = N_ASSIGNMENT;
    node.data.assignment.name = name;
    node.data.assignment.expression = expression;

    return node;
}

#endif