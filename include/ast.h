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

typedef struct Scope Scope;
typedef struct AST AST;
typedef enum ASTType ASTType;
typedef enum ASTFlag {
    ENUM_FLAG   = 1 << 0,
    PACKED_DATA = 1 << 1,
    ARGUMENT    = 1 << 2,
} ASTFlag;

struct varDef {AST* dataType; AST* identifier; AST* expression;} varDef;
struct structDef {AST* identifier; AST* members;} structDef;
struct enumDef {AST* identifier; AST* dataType; AST* constants;} enumDef;
struct funcDef {AST* returnType; AST* identifier; AST* arguments; AST* statements;} funcDef;
struct unionDef {AST* identifier; AST* members;} unionDef;
struct assign {AST* left; AST* right;} assign;
struct binop {AST* left; Token* operator; AST* right;} binop;

struct packed AST {
    enum packed ASTType {
        AST_TYPE, AST_ID, AST_COMP, AST_VALUE,
        AST_VAR, AST_STRUCT, AST_ENUM, AST_FUNC,
        AST_BINOP, AST_RETURN, AST_ASSIGN, AST_IMPORT,
        AST_C, AST_INTEGER, AST_UNION, AST_FUNC_VAR,
    } type;
    ASTFlag flags;
    union {
        DynArray* array;
        Token* token;
        size_t value;
        AST* expression;
        struct varDef* varDef;
        struct structDef* structDef;
        struct enumDef* enumDef;
        struct funcDef* funcDef;
        struct unionDef* unionDef;
        struct assign* assign;
        struct binop* binop;
    };
};
#define initAST(TYPE, FLAGS) RALLOC(1, sizeof(AST)); *_AST = (AST) {.type = TYPE, .flags = FLAGS}; AST_NODES_CONSTRUCTED++
#define valueAST(TYPE, FLAGS, MEMBER, ...) ({AST* _AST = initAST(TYPE, FLAGS); _AST->MEMBER = __VA_ARGS__; _AST;})
#define structAST(TYPE, FLAGS, MEMBER, ...) ({ \
AST* _AST = initAST(TYPE, FLAGS);              \
_AST->MEMBER = RALLOC(1, sizeof(MEMBER));      \
*_AST->MEMBER = (struct MEMBER) {__VA_ARGS__}; \
_AST; \
})

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