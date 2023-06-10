#ifndef LAVA_DEBUG_H
#define LAVA_DEBUG_H

#include <stdlib.h>
#include "ast.h"
#include "file.h"

const char* TOKEN_NAMES[] = {
    "End of File",
    "Unexpected Token",
    
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
    "True",
    "False",

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
    "AST Identifier",
    "AST Compound",
    "AST Binary Operator",
    "AST Integer",
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

void buildTokensAndPrint(char* fileName) {
    //Init lexer with file contents specified by arg 1
    Lexer* lex = lexerInit(read_file(fileName));

    List* tokens = listInit(sizeof(Token));
    while (1) {
        Token* token = lexNextToken(lex);
        if (token->type == TOKEN_UNEXPECTED) {
            //Could not tokenize current character, either invalid source or missed case.
            fprintf(stderr, "Unexpected Token(%d): %s\n", lex->pos, token->value);
            break;
        }
        listAppend(tokens, token);
        if (token->type == TOKEN_EOF) {
            break;
        }
    }

    for (size_t i = 0; i < tokens->len; i++) {
        Token* token = tokens->elements[i];
        printf("Token: %s: %s\n", TOKEN_NAMES[token->type], token->value);
    }
    printf("List: %d\n", tokens->len);

    //Free allocations
    listFree(tokens);
    lexerFree(lex);
}
#endif