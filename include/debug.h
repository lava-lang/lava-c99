#ifndef LAVA_DEBUG_H
#define LAVA_DEBUG_H

#include <stdlib.h>
#include "ast.h"
#include "file.h"

const char* TOKEN_NAMES[] = {
    "End of File",
    "Unexpected Token",
    "None",
    "C Statement",
    
    "NULL",
    "Void Type",
    "Int",
    "Int 8",
    "Int 16",
    "Int 32",
    "Int 64",
    "UInt 8",
    "UInt 16",
    "UInt 32",
    "UInt 64",
    "Integer Value",
    "Float",
    "Float 32",
    "Float 64",
    "Float Value",
    "String Type",
    "String Value",
    "Char Type",
    "Char Value",
    "Boolean Type",
    "Boolean Value",

    "Struct Definition",
    "Enum Definition",
    "If Statement",
    "Else Statement",
    "While Loop",
    "For Loop",
    "Return Statement",
    "Import Statement",

    "Identifier",
    "End of Statement",
    
    "Assignment Operator",
    "Equality Operator",
    
    "Division Operator",
    "Plus Operator",
    "Minus Operator",
    "Multiply Operator",
    "Less Than Operator",
    "More Than Operator",
    "Not Operator",

    "Left Paren",
    "Right Paren",
    "Colon",
    "Comma",
    "Dot",
    "Left Bracket",
    "Right Bracket",
    "Left Brace",
    "Right Brace",

    "Comment Line",
    "Comment Multi Line",
};
const char* AST_NAMES[] = {
    "AST Data Type",
    "AST Variable Definition",
    "AST Struct Definition",
    "AST Function Definition",
    "AST Identifier",
    "AST Compound",
    "AST Binary Operator",
    "AST Var Value",
    "AST C Statement",
    "AST Return",
    "AST Import",
};

void printAST(AST* node, int depth);

char* getIndent(int depth) {
    char* indent = mallocStr("");
    for (int i = 0; i < depth; ++i) {
        indent = concatStr(indent, "    ");
    }
    return indent;
}

void printCompound(ASTCompound* node, int depth) {
    for (int i = 0; i < node->children->len; ++i) {
        printAST(node->children->elements[i], depth + 1);
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
        printf("%sNode: %s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: %s\n", getIndent(depth + 1), varDef->identifier->token->value);
        printf("%sDataType: %s\n", getIndent(depth + 1), TOKEN_NAMES[varDef->dataType->token->type]);
        printExpression(varDef->expression, depth + 1);
    } else if (node->astType == AST_FUNC_DEF) {
        ASTFuncDef* funcDef = (ASTFuncDef*) node;
        printf("%sNode: %s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: %s\n", getIndent(depth + 1), funcDef->identifier->token->value);
        printf("%sReturnType: %s\n", getIndent(depth + 1), TOKEN_NAMES[funcDef->returnType->token->type]);
        printf("%sCompound: \n", getIndent(depth + 1));
        printCompound(funcDef->statements, depth + 1);
    } else if (node->astType == AST_STRUCT_DEF) {
        ASTStructDef* structDef = (ASTStructDef*) node;
        printf("%sNode: %s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sIdentifier: %s\n", getIndent(depth + 1), structDef->identifier->token->value);
        printf("%sCompound: \n", getIndent(depth + 1));
        printCompound(structDef->members, depth + 1);
    } else if (node->astType == AST_RETURN) {
        ASTReturn* returnAst = (ASTReturn*) node;
        printf("%sNode: %s\n", getIndent(depth), AST_NAMES[node->astType]);
        printExpression(returnAst->expression, depth + 1);
    } else if (node->astType == AST_C_STATEMENT) {
        printf("%sNode: %s\n", getIndent(depth), AST_NAMES[node->astType]);
        printf("%sValue: %s\n", getIndent(depth + 1), node->token->value);
    }
}
#endif