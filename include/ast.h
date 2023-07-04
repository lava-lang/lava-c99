#ifndef LAVA_AST_H
#define LAVA_AST_H

#include "token.h"
#include "structures.h"

static int AST_NODES_CONSTRUCTED = 0;

typedef enum ASTType {
    AST_DATA_TYPE,
    AST_VAR_DEF,
    AST_FUNC_DEF,
    AST_IDENTIFIER,
    AST_COMPOUND,
    AST_BINARY_OP,
    AST_VAR_VALUE,
    AST_C_STATEMENT,
    AST_RETURN,
    AST_IMPORT,
} ASTType;

typedef struct Scope Scope;
typedef struct AST AST;

typedef struct Scope {
    AST* ast;
} Scope;

Scope* scopeInit(AST* ast) {
    Scope* scope = calloc(1, sizeof(Scope));
    scope->ast = ast;
    return scope;
}

void scopeFree(Scope* scope) {
    if (scope->ast) {
        free(scope->ast);
    }
    free(scope);
}

typedef struct AST {
    int astType;
    Token* token;
    Scope* scope;
} AST;

AST* initASTBase(Token* token, AST* ast, ASTType astType) {
    ast->token = token;
    ast->astType = astType;
    ast->scope = (void*) 0;
    AST_NODES_CONSTRUCTED++;
    return ast;
}

AST* initAST(Token* token, ASTType astType) {
    return initASTBase(token, calloc(1, sizeof(AST)), astType);
}

typedef struct ASTCompound {
    AST* base;
    List* children;
} ASTCompound;

AST* initASTCompound(Token* token, List* children) {
    ASTCompound* compound = calloc(1, sizeof(ASTCompound));
    initASTBase(token, (AST*) compound, AST_COMPOUND);
    compound->children = children;
    return (AST*) compound;
}

typedef struct ASTVarDef {
    AST base;
    AST* dataType;
    AST* identifier;
    AST* expression;
} ASTVarDef;

AST* initASTVarDef(AST* dataType, AST* identifier, AST* expression) {
    ASTVarDef* varDef = calloc(1, sizeof(ASTVarDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) varDef, AST_VAR_DEF);
    varDef->dataType = dataType;
    varDef->identifier = identifier;
    varDef->expression = expression;
    return (AST*) varDef;
}

typedef struct ASTFuncDef {
    AST base;
    AST* returnType;
    AST* identifier;
    AST* compound;
} ASTFuncDef;

AST* initASTFuncDef(AST* returnType, AST* identifier, AST* compound) {
    ASTFuncDef* funcDef = calloc(1, sizeof(ASTFuncDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) funcDef, AST_FUNC_DEF);
    funcDef->returnType = returnType;
    funcDef->identifier = identifier;
    funcDef->compound = compound;
    return (AST*) funcDef;
}

typedef struct ASTBinaryOp {
    AST* base;
    AST* left;
    AST* right;
} ASTBinaryOp;

AST* initASTBinaryOp(Token* token, AST* left, AST* right) {
    ASTBinaryOp* binaryOp = calloc(1, sizeof(ASTBinaryOp));
    initASTBase(token, (AST*) binaryOp, AST_BINARY_OP);
    binaryOp->left = left;
    binaryOp->right = right;
    return (AST*) binaryOp;
}

typedef struct ASTReturn {
    AST* base;
    AST* expression;
} ASTReturn;

AST* initASTReturn(AST* expression) {
    ASTReturn* returnStatement = calloc(1, sizeof(ASTReturn));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) returnStatement, AST_RETURN);
    returnStatement->expression = expression;
    return (AST*) returnStatement;
}

#endif //LAVA_AST_H