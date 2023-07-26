#ifndef LAVA_DEBUG_H
#define LAVA_DEBUG_H

#include <stdlib.h>
#include "ast.h"
#include "file.h"

void printAST(AST* node, int depth);

char* getIndent(int depth) {
    char* indent = mallocStr("");
    for (int i = 0; i < depth; ++i) {
        indent = concatStr(indent, "    ");
    }
    return indent;
}

void printCompound(AST* node, int depth) {
    for (int i = 0; i < node->array->len; ++i) {
        printAST(node->array->elements[i], depth + 1);
    }
}

void printExpression(AST* node, int depth) {
    if (node == NULL) return;
    if (node->type == AST_BINOP) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printExpression(node->dualDef.left, depth + 1);
        printf("%sOperator: ", getIndent(depth + 1)); printView(&node->token->view, "\n");
        printExpression(node->dualDef.right, depth + 1);
    } else if (node->type == AST_VALUE) {
        printf("%sValue: ", getIndent(depth)); printView(&node->token->view, "\n");
    }
}

void printAST(AST* node, int depth) {
    if (node->type == AST_VAR) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&node->varDef.identifier->token->view, "\n");
        printf("%sDataType: %s\n", getIndent(depth + 1), TOKEN_NAMES[node->varDef.dataType->token->type]);
        printExpression(node->varDef.expression, depth + 1);
    } else if (node->type == AST_FUNC) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&node->funcDef.identifier->token->view, "\n");
        printf("%sReturnType: %s\n", getIndent(depth + 1), TOKEN_NAMES[node->funcDef.returnType->token->type]);
        printf("%sArguments: \n", getIndent(depth + 1));
        printCompound(node->funcDef.arguments, depth + 1);
        printf("%sStatements: \n", getIndent(depth + 1));
        printCompound(node->funcDef.statements, depth + 1);
    } else if (node->type == AST_STRUCT) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&node->idComp.identifier->token->view, "\n");
        printf("%sMembers: \n", getIndent(depth + 1));
        printCompound(node->idComp.members, depth + 1);
    } else if (node->type == AST_RETURN) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printExpression(node->expression, depth + 1);
    } else if (node->type == AST_C) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sValue: ", getIndent(depth + 1)); printView(&node->token->view, "\n");
    }
}
#endif