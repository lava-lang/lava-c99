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

void printCompound(ASTCompound* node, int depth) {
    for (int i = 0; i < node->array->len; ++i) {
        printAST(node->array->elements[i], depth + 1);
    }
}

void printExpression(AST* node, int depth) {
    if (node == NULL) return;
    if (node->astType == AST_BINARY_OP) {
        ASTBinaryOp* binOp = (ASTBinaryOp*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printExpression(binOp->left, depth + 1);
        printf("%sOperator: ", getIndent(depth + 1)); printView(&binOp->base.token->view, "\n");
        printExpression(binOp->right, depth + 1);
    } else if (node->astType == AST_VAR_VALUE) {
        printf("%sValue: ", getIndent(depth)); printView(&node->token->view, "\n");
    }
}

void printAST(AST* node, int depth) {
    if (node->astType == AST_VAR_DEF) {
        ASTVarDef* varDef = (ASTVarDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&varDef->identifier->token->view, "\n");
        printf("%sDataType: %s\n", getIndent(depth + 1), TOKEN_NAMES[varDef->dataType->token->type]);
        printExpression(varDef->expression, depth + 1);
    } else if (node->astType == AST_FUNC_DEF) {
        ASTFuncDef* funcDef = (ASTFuncDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&funcDef->identifier->token->view, "\n");
        printf("%sReturnType: %s\n", getIndent(depth + 1), TOKEN_NAMES[funcDef->returnType->token->type]);
        printf("%sArguments: \n", getIndent(depth + 1));
        printCompound(funcDef->arguments, depth + 1);
        printf("%sStatements: \n", getIndent(depth + 1));
        printCompound(funcDef->statements, depth + 1);
    } else if (node->astType == AST_STRUCT_DEF) {
        ASTStructDef* structDef = (ASTStructDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: ", getIndent(depth + 1)); printView(&structDef->identifier->token->view, "\n");
        printf("%sMembers: \n", getIndent(depth + 1));
        printCompound(structDef->members, depth + 1);
    } else if (node->astType == AST_RETURN) {
        ASTReturn* returnAst = (ASTReturn*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printExpression(returnAst->expression, depth + 1);
    } else if (node->astType == AST_C_STATEMENT) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sValue: ", getIndent(depth + 1)); printView(&node->token->view, "\n");
    }
}
#endif