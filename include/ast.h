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
    Token* token;
    ASTFlag flags;
    union {
        struct comp {
            DynArray* array;
        } comp;
        struct varDef {
            AST* dataType;
            AST* identifier;
            AST* expression;
        } varDef;
        struct structDef {
            AST* identifier;
            AST* members;
        } structDef;
        struct enumDef {
            AST* identifier;
            AST* constants;
        } enumDef;
        struct funcDef {
            AST* returnType;
            AST* identifier;
            AST* arguments;
            AST* statements;
        } funcDef;
        struct dualDef {
            AST* left;
            AST* right;
        } dualDef;
        struct expr {
            AST* node;
        } expr;
        struct integer {
            size_t value;
        } integer;
    };
};

AST* initAST(Token* token, ASTType type, ASTFlag flags) {
    AST* ast = RALLOC(1, sizeof(AST));
    *ast = (AST) {.token = token, .type = type, .flags = flags};
    AST_NODES_CONSTRUCTED++;
    //DEBUG("CONSTRUCTED AST: %s\n", AST_NAMES[astType]);
    return ast;
}

#define initSetAST(TOK, TYPE, FLAGS, MEMBER, ...) ({AST* _AST = initAST(TOK, TYPE, FLAGS); _AST->MEMBER = (struct MEMBER) {__VA_ARGS__}; _AST;})

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
#endif //LAVA_AST_H