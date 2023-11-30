#ifndef LAVA_DEBUG_H
#define LAVA_DEBUG_H

#include <stdlib.h>
#include "ast_old.h"
#include "file.h"

void printAST(AST* node, int depth);

char* getIndent(int depth) {
    char* indent = mallocStr("");
    for (int i = 0; i < depth; ++i) {
        indent = concatStr(indent, "    ");
    }
    return indent;
}

void printCompound(ASTComp* node, int depth) {
    for (int i = 0; i < node->array->len; ++i) {
        printAST(node->array->elements[i], depth + 1);
    }
}

void printExpression(AST* node, int depth) {
    if (node == NULL) return;
    if (node->type == AST_BINOP) {
        ASTBinop* binop = (ASTBinop*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printExpression(binop->left, depth + 1);
        printf("%sOperator: ", getIndent(depth + 1)); printView(&binop->op.view, "\n");
        printExpression(binop->right, depth + 1);
    } else if (node->type == AST_VALUE) {
        printf("%sValue: ", getIndent(depth)); printView(&node->token.view, "\n");
    }
}

void printAST(AST* node, int depth) {
    if (node->type == AST_VAR) {
        ASTVarDef* varDef = (ASTVarDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&varDef->identifier->token.view, "\n");
        printf("%sDataType: %s\n", getIndent(depth + 1), TOKEN_NAMES[varDef->dataType->token.type]);
        printExpression(varDef->expression, depth + 1);
    } else if (node->type == AST_FUNC) {
        ASTFuncDef* funcDef = (ASTFuncDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&funcDef->identifier->token.view, "\n");
        printf("%sReturnType: %s\n", getIndent(depth + 1), TOKEN_NAMES[funcDef->returnType->token.type]);
        printf("%sArguments: \n", getIndent(depth + 1));
        printCompound(funcDef->arguments, depth + 1);
        printf("%sStatements: \n", getIndent(depth + 1));
        printCompound(funcDef->statements, depth + 1);
    } else if (node->type == AST_STRUCT) {
        ASTStructDef* structDef = (ASTStructDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&structDef->identifier->token.view, "\n");
        printf("%sMembers: \n", getIndent(depth + 1));
        printCompound(structDef->members, depth + 1);
    } else if (node->type == AST_RETURN) {
        ASTExpr* retDef = (ASTExpr*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printExpression(retDef->expr, depth + 1);
    } else if (node->type == AST_C) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->type]);
        printf("%sValue: ", getIndent(depth + 1)); printView(&node->token.view, "\n");
    }
}
#endif