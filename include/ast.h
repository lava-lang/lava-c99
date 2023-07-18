#ifndef LAVA_AST_H
#define LAVA_AST_H

#include "token.h"
#include "debug.h"
#include "lexer.h"
#include "region.h"

static int AST_NODES_CONSTRUCTED = 0;

const char* AST_NAMES[] = {
        "Data Type",
        "Variable Definition",
        "Struct Definition",
        "Function Definition",
        "Identifier",
        "Compound",
        "Binary Operator",
        "Var Value",
        "C Statement",
        "Return",
        "Import",
};

typedef enum ASTType {
    AST_DATA_TYPE,
    AST_VAR_DEF,
    AST_STRUCT_DEF,
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
//    Scope* scope = CALLOC(1, sizeof(Scope));
//    scope->ast = ast;
//    return scope;
    return NULL;
}

void scopeFree(Scope* scope) {
    if (scope && scope->ast) {
        //FREE(scope->ast);
    }
    FREE(scope);
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
    DEBUG("CONSTRUCTED AST: %s\n", AST_NAMES[astType]);
    return ast;
}

AST* initAST(Token* token, ASTType astType) {
    return initASTBase(token, RALLOC(1, sizeof(AST)), astType);
}

typedef struct ASTCompound {
    AST* base;
    DynArray* array;
} ASTCompound;

AST* initASTCompound(DynArray* array) {
    ASTCompound* compound = RALLOC(1, sizeof(ASTCompound));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) compound, AST_COMPOUND);
    compound->array = array;
    return (AST*) compound;
}

typedef struct ASTVarDef {
    AST base;
    AST* dataType;
    AST* identifier;
    AST* expression;
} ASTVarDef;

AST* initASTVarDef(AST* dataType, AST* identifier, AST* expression) {
    ASTVarDef* varDef = RALLOC(1, sizeof(ASTVarDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) varDef, AST_VAR_DEF);
    varDef->dataType = dataType;
    varDef->identifier = identifier;
    varDef->expression = expression;
    return (AST*) varDef;
}

typedef struct ASTStructDef {
    AST base;
    AST* identifier;
    ASTCompound* members;
} ASTStructDef;

AST* initASTStructDef(AST* identifier, AST* members) {
    ASTStructDef* structDef = RALLOC(1, sizeof(ASTStructDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) structDef, AST_STRUCT_DEF);
    structDef->identifier = identifier;
    structDef->members = (ASTCompound*) members;
    return (AST*) structDef;
}

typedef struct ASTFuncDef {
    AST base;
    AST* returnType;
    AST* identifier;
    ASTCompound* arguments;
    ASTCompound* statements;
} ASTFuncDef;

AST* initASTFuncDef(AST* returnType, AST* identifier, AST* arguments, AST* statements) {
    ASTFuncDef* funcDef = RALLOC(1, sizeof(ASTFuncDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) funcDef, AST_FUNC_DEF);
    funcDef->returnType = returnType;
    funcDef->identifier = identifier;
    funcDef->arguments = (ASTCompound*) arguments;
    funcDef->statements = (ASTCompound*) statements;
    return (AST*) funcDef;
}

typedef struct ASTBinaryOp {
    AST* base;
    AST* left;
    AST* right;
} ASTBinaryOp;

AST* initASTBinaryOp(AST* left, AST* right) {
    ASTBinaryOp* binaryOp = RALLOC(1, sizeof(ASTBinaryOp));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) binaryOp, AST_BINARY_OP);
    binaryOp->left = left;
    binaryOp->right = right;
    return (AST*) binaryOp;
}

typedef struct ASTReturn {
    AST* base;
    AST* expression;
} ASTReturn;

AST* initASTReturn(AST* expression) {
    ASTReturn* returnStatement = RALLOC(1, sizeof(ASTReturn));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) returnStatement, AST_RETURN);
    returnStatement->expression = expression;
    return (AST*) returnStatement;
}
#endif //LAVA_AST_H