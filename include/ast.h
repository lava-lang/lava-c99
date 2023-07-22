#ifndef LAVA_AST_H
#define LAVA_AST_H

#include "token.h"
#include "debug.h"
#include "lexer.h"
#include "region.h"

static int AST_NODES_CONSTRUCTED = 0;

const char* AST_NAMES[] = {
        "Integer",
        "Data Type",
        "Variable Definition",
        "Struct Definition",
        "Enum Definition",
        "Function Definition",
        "Identifier",
        "Compound",
        "Binary Operator",
        "Var Value",
        "Assigment",
        "C Statement",
        "Return",
        "Import",
};

typedef enum ASTType {
    AST_INTEGER,
    AST_DATA_TYPE,
    AST_VAR_DEF,
    AST_STRUCT_DEF,
    AST_ENUM_DEF,
    AST_FUNC_DEF,
    AST_IDENTIFIER,
    AST_COMPOUND,
    AST_BINARY_OP,
    AST_VAR_VALUE,
    AST_ASSIGNMENT,
    AST_C_STATEMENT,
    AST_RETURN,
    AST_IMPORT,
} ASTType;

typedef enum ASTFlag {
    TODO = 1 << 0,
} ASTFlag;

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
    size_t astType;
    size_t flags;
    Token* token;
    Scope* scope;
} AST;

AST* initASTBase(Token* token, AST* ast, ASTType astType, ASTFlag flags) {
    ast->token = token;
    ast->astType = astType;
    ast->flags = flags;
    ast->scope = (void*) 0;
    AST_NODES_CONSTRUCTED++;
    //DEBUG("CONSTRUCTED AST: %s\n", AST_NAMES[astType]);
    return ast;
}

AST* initAST(Token* token, ASTType type, ASTFlag flags) {
    return initASTBase(token, RALLOC(1, sizeof(AST)), type, flags);
}

typedef struct ASTInteger {
    AST base;
    size_t value;
} ASTInteger;

AST* initASTInteger(size_t value) {
    ASTInteger* integer = RALLOC(1, sizeof(ASTInteger));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) integer, AST_INTEGER, 0);
    integer->value = value;
    return (AST*) integer;
}

typedef struct ASTCompound {
    AST base;
    DynArray* array;
} ASTCompound;

AST* initASTCompound(DynArray* array) {
    ASTCompound* compound = RALLOC(1, sizeof(ASTCompound));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) compound, AST_COMPOUND, 0);
    compound->array = array;
    return (AST*) compound;
}

typedef struct ASTAssignment {
    AST base;
    AST* left;
    AST* right;
} ASTAssignment;

AST* initASTAssignment(AST* left, AST* right) {
    ASTAssignment* assignment = RALLOC(1, sizeof(ASTAssignment));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) assignment, AST_ASSIGNMENT, 0);
    assignment->left = left;
    assignment->right = right;
    return (AST*) assignment;
}

typedef struct ASTVarDef {
    AST base;
    AST* dataType;
    AST* identifier;
    AST* expression;
} ASTVarDef;

AST* initASTVarDef(AST* dataType, AST* identifier, AST* expression) {
    ASTVarDef* varDef = RALLOC(1, sizeof(ASTVarDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) varDef, AST_VAR_DEF, 0);
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
    initASTBase(&STATIC_TOKEN_NONE, (AST*) structDef, AST_STRUCT_DEF, 0);
    structDef->identifier = identifier;
    structDef->members = (ASTCompound*) members;
    return (AST*) structDef;
}

typedef struct ASTEnumDef {
    AST base;
    AST* identifier;
    ASTCompound* constants;
} ASTEnumDef;

AST* initASTEnumDef(AST* identifier, AST* constants, ASTFlag flags) {
    ASTEnumDef* enumDef = RALLOC(1, sizeof(ASTEnumDef));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) enumDef, AST_ENUM_DEF, flags);
    enumDef->identifier = identifier;
    enumDef->constants = (ASTCompound*) constants;
    return (AST*) enumDef;
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
    initASTBase(&STATIC_TOKEN_NONE, (AST*) funcDef, AST_FUNC_DEF, 0);
    funcDef->returnType = returnType;
    funcDef->identifier = identifier;
    funcDef->arguments = (ASTCompound*) arguments;
    funcDef->statements = (ASTCompound*) statements;
    return (AST*) funcDef;
}

typedef struct ASTBinaryOp {
    AST base;
    AST* left;
    AST* right;
} ASTBinaryOp;

AST* initASTBinaryOp(Token* token, AST* left, AST* right) {
    ASTBinaryOp* binaryOp = RALLOC(1, sizeof(ASTBinaryOp));
    initASTBase(token, (AST*) binaryOp, AST_BINARY_OP, 0);
    binaryOp->left = left;
    binaryOp->right = right;
    return (AST*) binaryOp;
}

typedef struct ASTReturn {
    AST base;
    AST* expression;
} ASTReturn;

AST* initASTReturn(AST* expression) {
    ASTReturn* returnStatement = RALLOC(1, sizeof(ASTReturn));
    initASTBase(&STATIC_TOKEN_NONE, (AST*) returnStatement, AST_RETURN, 0);
    returnStatement->expression = expression;
    return (AST*) returnStatement;
}
#endif //LAVA_AST_H