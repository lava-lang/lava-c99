#ifndef LAVA_AST_H
#define LAVA_AST_H

#include "token.h"
#include "debug.h"
#include "lexer.h"
#include "region.h"

static int AST_NODES_CONSTRUCTED = 0;

const char* AST_NAMES[] = {
    "Data Type", "Identifier", "Compound", "Var Value",
    "Variable Definition", "Struct Definition", "Enum Definition", "Function Definition",
    "Binary Operator", "Return", "Assigment", "Import",
    "C Statement", "Integer", "Union", "Variable Function",
};

typedef enum packed ASTType {
    AST_TYPE, AST_ID, AST_COMP, AST_VALUE,
    AST_VAR, AST_STRUCT, AST_ENUM, AST_FUNC,
    AST_BINOP, AST_RETURN, AST_ASSIGN, AST_IMPORT,
    AST_C, AST_INTEGER, AST_UNION, AST_FUNC_VAR,
} ASTType;

typedef enum packed ASTFlag {
    ENUM_FLAG   = 1 << 0,
    PACKED_DATA = 1 << 1,
    ARGUMENT    = 1 << 2,
} ASTFlag;

typedef struct AST {
    ASTType type;
    ASTFlag flags;
    Token token;
} AST;

typedef struct ASTComp {
    AST base;
    DynArray* array;
} ASTComp;

typedef struct ASTLiteral {
    AST base;
    union {
        size_t value;
    };
} ASTLiteral;

typedef struct ASTVarDef {
    AST base;
    AST* dataType;
    AST* identifier;
    AST* expression;
} ASTVarDef;

typedef struct ASTStructDef {
    AST base;
    AST* identifier;
    ASTComp* members;
} ASTStructDef;

typedef struct ASTEnumDef {
    AST base;
    AST* identifier;
    AST* dataType;
    ASTComp* constants;
} ASTEnumDef;

typedef struct ASTFuncDef {
    AST base;
    AST* returnType;
    AST* identifier;
    ASTComp* arguments;
    ASTComp* statements;
} ASTFuncDef;

typedef struct ASTUnionDef {
    AST base;
    AST* identifier;
    ASTComp* members;
} ASTUnionDef;

typedef struct ASTAssign {
    AST base;
    AST* left;
    AST* right;
} ASTAssign;

typedef struct ASTBinop {
    AST base;
    AST* left;
    Token op;
    AST* right;
} ASTBinop;

typedef struct ASTExpr {
    AST base;
    AST* expr;
} ASTExpr;

#define initAST(TYPE, FLAGS, STRUCT) RALLOC(1, sizeof(STRUCT)); AST_NODES_CONSTRUCTED++
#define basicAST(TYPE, FLAGS, TOK) ({AST* _AST = initAST(TYPE, FLAGS, AST); *_AST = (struct AST) {TYPE, FLAGS, {.type = TOK.type, .flags = TOK.flags, .view = TOK.view}}; _AST;})
#define valueAST(TYPE, FLAGS, MEMBER, VALUE) ({ASTLiteral* _AST = initAST(TYPE, FLAGS, ASTLiteral); _AST->base.type = TYPE; _AST->base.flags = FLAGS; _AST->MEMBER = VALUE; _AST;})
#define structAST(TYPE, FLAGS, STRUCT, ...) ({STRUCT* _AST = initAST(TYPE, FLAGS, STRUCT); *_AST = (struct STRUCT) {TYPE, FLAGS, {0}, __VA_ARGS__}; _AST;})

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
    FREE(scope);
}
#endif //LAVA_AST_H