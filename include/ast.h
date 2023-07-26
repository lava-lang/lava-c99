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
    "C Statement", "Integer",
};

typedef struct AST AST;
typedef enum ASTType ASTType;
typedef enum ASTFlag {
    TODO = 1 << 0,
} ASTFlag;

typedef struct Scope Scope;

struct AST {
    enum ASTType {
        AST_TYPE, AST_ID, AST_COMP, AST_VALUE,
        AST_VAR, AST_STRUCT, AST_ENUM, AST_FUNC,
        AST_BINOP, AST_RETURN, AST_ASSIGN, AST_IMPORT,
        AST_C, AST_INTEGER,
    } type;
    ASTFlag flags;
    union {
        DynArray* array;
        Token* token;
        size_t value;
        AST* expression;
        struct varDef {AST* dataType; AST* identifier; AST* expression;} varDef;
        struct structDef {AST* identifier; AST* members;} structDef;
        struct enumDef {AST* identifier; AST* constants;} enumDef;
        struct funcDef {AST* returnType; AST* identifier; AST* arguments; AST* statements;} funcDef;
        struct assign {AST* left; AST* right;} assign;
        struct binop {AST* left; Token* operator; AST* right;} binop;
    };
};
#define initAST(TYPE, FLAGS) RALLOC(1, sizeof(AST)); *_AST = (AST) {.type = TYPE, .flags = FLAGS}; AST_NODES_CONSTRUCTED++
#define valueAST(TYPE, FLAGS, MEMBER, ...) ({AST* _AST = initAST(TYPE, FLAGS); _AST->MEMBER = __VA_ARGS__; _AST;})
#define structAST(TYPE, FLAGS, MEMBER, ...) ({AST* _AST = initAST(TYPE, FLAGS); _AST->MEMBER = (struct MEMBER) {__VA_ARGS__}; _AST;})

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