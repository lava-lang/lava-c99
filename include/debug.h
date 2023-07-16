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
    //TODO this should not be null
    if (node) {
        printf("%sExpression: %s\n", getIndent(depth), node->token->value);
    }
}

void printAST(AST* node, int depth) {
    if (node->astType == AST_VAR_DEF) {
        ASTVarDef* varDef = (ASTVarDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: %s\n", getIndent(depth + 1), varDef->identifier->token->value);
        printf("%sDataType: %s\n", getIndent(depth + 1), TOKEN_NAMES[varDef->dataType->token->type]);
        printExpression(varDef->expression, depth + 1);
    } else if (node->astType == AST_FUNC_DEF) {
        ASTFuncDef* funcDef = (ASTFuncDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: %s\n", getIndent(depth + 1), funcDef->identifier->token->value);
        printf("%sReturnType: %s\n", getIndent(depth + 1), TOKEN_NAMES[funcDef->returnType->token->type]);
        printf("%sArguments: \n", getIndent(depth + 1));
        printCompound(funcDef->arguments, depth + 1);
        printf("%sStatements: \n", getIndent(depth + 1));
        printCompound(funcDef->statements, depth + 1);
    } else if (node->astType == AST_STRUCT_DEF) {
        ASTStructDef* structDef = (ASTStructDef*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: %s\n", getIndent(depth + 1), structDef->identifier->token->value);
        printf("%sMembers: \n", getIndent(depth + 1));
        printCompound(structDef->members, depth + 1);
    } else if (node->astType == AST_RETURN) {
        ASTReturn* returnAst = (ASTReturn*) node;
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printExpression(returnAst->expression, depth + 1);
    } else if (node->astType == AST_C_STATEMENT) {
        printf("%s%s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sValue: %s\n", getIndent(depth + 1), node->token->value);
    }
}
#endif