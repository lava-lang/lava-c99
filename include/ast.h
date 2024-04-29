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
    "Binary Operator", "Unary Operator", "Return", "Assigment", "Import",
    "C Statement", "Integer", "Union", "Variable Function",
    "If Statement", "Else Statement", "While Loop", "For Loop", "Expression", "Function Call",
    "Struct Init", "Struct Member Ref", "Break Statement", "Array Init",
    "Array Access",
};

//TODO AST nodes should have their parent as a member, to avoid
//TODO requiring that logic within parser and cgen

typedef struct AST AST;
typedef enum packed ASTType {
    AST_TYPE, AST_ID, AST_COMP, AST_VALUE,
    AST_VAR, AST_STRUCT, AST_ENUM, AST_FUNC,
    AST_BINOP, AST_UNARY, AST_RETURN, AST_ASSIGN, AST_IMPORT,
    AST_C, AST_INTEGER, AST_UNION, AST_FUNC_VAR,
    AST_IF, AST_ELSE, AST_WHILE, AST_FOR, AST_EXPR, AST_FUNC_CALL,
    AST_STRUCT_INIT, AST_STRUCT_MEMBER_REF, AST_BREAK,
    AST_ARRAY_INIT, AST_ARRAY_ACCESS,
} ASTType;
typedef enum packed ASTFlag {
    ENUM_FLAG     = 1 << 0,
    PACKED_DATA   = 1 << 1,
    ARGUMENT      = 1 << 2,
    STRUCT_FUNC   = 1 << 3,
    STRUCT_MEMBER = 1 << 4,
    UNARY_LEFT    = 1 << 5,
    UNARY_RIGHT   = 1 << 6,
    NON_EXPR_FUNC = 1 << 7,
    POINTER_TYPE  = 1 << 8,
    REF_TYPE      = 1 << 9,
    DEREF_TYPE    = 1 << 10,
    ARRAY_TYPE    = 1 << 11,
    TRAILING_EOS  = 1 << 12,
} ASTFlag;
struct AST {
    ASTType type;
    ASTFlag flags;
    Token* token;
};
typedef struct ASTComp {AST base; DynArray* array;} ASTComp;
typedef struct ASTLiteral {AST base; union {size_t value; };} ASTLiteral;
typedef struct ASTVarDef {AST base; AST* dataType; AST* identifier; AST* expression;} ASTVarDef;
typedef struct ASTStructDef {AST base; AST* identifier; ASTComp* members;} ASTStructDef;
typedef struct ASTEnumDef {AST base; AST* identifier; AST* dataType; ASTComp* constants;} ASTEnumDef;
typedef struct ASTFuncDef {AST base; AST* returnType; AST* identifier; ASTComp* arguments; ASTComp* statements; char* structIden;} ASTFuncDef;
typedef struct ASTUnionDef {AST base; AST* identifier; ASTComp* members;} ASTUnionDef;
typedef struct ASTAssign {AST base; AST* left; AST* right;} ASTAssign;
//TODO Token* in Binop can just be TokenType? avoids storing pointer where an int can suffice
typedef struct ASTBinop {AST base; AST* left; Token* op; AST* right;} ASTBinop;
typedef struct ASTUnary {AST base; AST* expression; Token* op;} ASTUnary;
typedef struct ASTExpr {AST base; AST* expr;} ASTExpr;
typedef struct ASTIf {AST base; AST* expr; ASTComp* body;} ASTIf;
typedef struct ASTElse {AST base; ASTComp* body;} ASTElse;
typedef struct ASTWhile {AST base; AST* expr; ASTComp* body;} ASTWhile;
typedef struct ASTFor {AST base; AST* definition; AST* condition; AST* expression; ASTComp* body;} ASTFor;
typedef struct ASTBreak {AST base; AST* expr; ASTComp* body;} ASTBreak;
typedef struct ASTFuncCall {AST base; AST* identifier; ASTComp* expressions; char* structIden;} ASTFuncCall;
typedef struct ASTStructInit {AST base; AST* identifier; ASTStructDef* structDef; ASTComp* expressions;} ASTStructInit;
typedef struct ASTStructMemberRef {AST base; AST* varIden; AST* memberIden;} ASTStructMemberRef;
typedef struct ASTArrayInit {AST base; AST* type; AST* identifier; AST* expression;} ASTArrayInit;
typedef struct ASTArrayAccess {AST base; AST* identifier; AST* expression;} ASTArrayAccess;

#define initAST(TYPE, FLAGS, STRUCT) RALLOC(1, sizeof(STRUCT)); AST_NODES_CONSTRUCTED++
#define basicAST(TYPE, FLAGS, TOK) ({AST* _AST = initAST(TYPE, FLAGS, AST); *_AST = (struct AST) {TYPE, FLAGS, TOK}; _AST;})
#define valueAST(TYPE, FLAGS, MEMBER, VALUE) ({ASTLiteral* _AST = initAST(TYPE, FLAGS, ASTLiteral); _AST->base.type = TYPE; _AST->base.flags = FLAGS; _AST->MEMBER = VALUE; _AST;})
#define structAST(TYPE, FLAGS, STRUCT, ...) ({STRUCT* _AST = initAST(TYPE, FLAGS, STRUCT); *_AST = (struct STRUCT) {TYPE, FLAGS, &TOKEN_NONE, __VA_ARGS__}; _AST;})

#endif //LAVA_AST_H