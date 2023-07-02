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
    "Int 32",
    "Int 32",
    "Int 64",
    "Integer Value",
    "Float 32",
    "Float 32",
    "Float 64",
    "Float Value",
    "String Type",
    "String Value",
    "Boolean Type",
    "Boolean Value",

    "Type Definition",
    "Enum Definition",
    "If Statement",
    "Else Statement",
    "While Loop",
    "For Loop",
    "Return Statement",

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
    "AST Function Definition",
    "AST Identifier",
    "AST Compound",
    "AST Binary Operator",
    "AST Var Value",
    "AST C Statement",
};

void printExpression(AST* node) {
    printf("        Type: %s\n", TOKEN_NAMES[node->token->type]);
    printf("        Value: %s\n", node->token->value);
}

void printVarDef(ASTVarDef* varDef) {
    printf("    DataType: %s\n", TOKEN_NAMES[varDef->dataType->token->type]);
    printf("    Identifier: %s\n", varDef->identifier->token->value);
    printf("    Expression: \n");
    printExpression(varDef->expression);
}

void printFuncDef(ASTFuncDef* funcDef) {
    printf("    ReturnType: %s\n", TOKEN_NAMES[funcDef->returnType->token->type]);
    printf("    Identifier: %s\n", funcDef->identifier->token->value);
    printf("    Compound: \n");
    //printCompound(funcDef->compound);
}
#endif