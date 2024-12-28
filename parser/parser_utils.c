#include <stdio.h>
#include <stdbool.h>
#include <wchar.h>
#include "parser.h"
#include "../src_lexer/lexer.h"

void print_ast_indent(int indent_level)
{
    for (int i = 0; i < indent_level; i++)
    {
        wprintf(L"\t");
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
        wprintf(L"Literal: ");
        if (node.data.literal.literal_type == LITERAL_FLOAT)
            wprintf(L"Float: %s\n", node.data.literal.value);
        else if (node.data.literal.literal_type == LITERAL_INT)
            wprintf(L"Int: %s\n", node.data.literal.value);
        else
            wprintf(L"Unknown Literal Type\n");
    }
    break;
    case N_UNARY_EXPRESSION:
    {
        wprintf(L"Unary: ");
        switch (node.data.unary.operator)
        {
        case T_PLUS:
            wprintf(L"+\n");
            break;
        case T_MINUS:
            wprintf(L"-\n");
            break;
        default:
            wprintf(L"Unknown Unary Operator\n");
        }
        print_ast_node(parser, node.data.unary.expression, indent_level + 1);
    }
    break;

    case N_BINARY_EXPRESSION:
    {
        wprintf(L"Binary: ");
        switch (node.data.binary.operator)
        {
        case T_PLUS:
            wprintf(L"+\n");
            break;
        case T_MINUS:
            wprintf(L"-\n");
            break;
        case T_MULTIPLY:
            wprintf(L"*\n");
            break;
        case T_DIVIDE:
            wprintf(L"/\n");
            break;
        default:
            wprintf(L"Unknown Binary Operator\n");
        }
        print_ast_indent(indent_level);
        wprintf(L"Left operand:\n");
        print_ast_node(parser, node.data.binary.left, indent_level + 1);

        print_ast_indent(indent_level);
        wprintf(L"Right operand:\n");
        print_ast_node(parser, node.data.binary.right, indent_level + 1);
    }
    break;

    default:
        wprintf(L"Unknown node type\n");
    }
}
