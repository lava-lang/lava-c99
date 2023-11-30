#ifndef LAVA_AST_H
#define LAVA_AST_H

typedef struct Statement Statement;
typedef struct Declaration Declaration;

typedef enum DeclarationType {
    DECL_NONE,
    DECL_ENUM,
    DECL_STRUCT,
    DECL_UNION,
    DECL_VAR,
    DECL_CONST,
    DECL_TYPEDEF,
    DECL_FUNC,
    DECL_NOTE,
    DECL_IMPORT,
} DeclarationType;

struct Declaration {
    DeclarationType type;
    const char* name;
    union {
        struct {
            //TODO
        } enumVal;
        struct {
            //TODO
        } funcVal;
        struct {
            //TODO
        } aliasVal;
        struct {
            //TODO
        } varVal;
        struct {
            //TODO
        } constVal;
        struct {
            //TODO
        } importVal;
    };
};

typedef enum packed ExpressionType {
    EXPRESSION_NONE,
    EXPRESSION_PAREN,
    EXPRESSION_INT,
    EXPRESSION_FLOAT,
    EXPRESSION_STR,
    EXPRESSION_NAME,
    EXPRESSION_CAST,
    EXPRESSION_CALL,
    EXPRESSION_INDEX,
    EXPRESSION_FIELD,
    EXPRESSION_COMPOUND,
    EXPRESSION_UNARY,
    EXPRESSION_BINARY,
    EXPRESSION_TERNARY,
    EXPRESSION_MODIFY,
    EXPRESSION_SIZEOF_EXPR,
    EXPRESSION_SIZEOF_TYPE,
    EXPRESSION_TYPEOF_EXPR,
    EXPRESSION_TYPEOF_TYPE,
    EXPRESSION_ALIGNOF_EXPR,
    EXPRESSION_ALIGNOF_TYPE,
    EXPRESSION_OFFSETOF,
    EXPRESSION_NEW,
} ExpressionType;

typedef struct Expression {

} Expression;

typedef enum packed StatementType {
    STATEMENT_NONE,
    STATEMENT_DECL,
    STATEMENT_RETURN,
    STATEMENT_BREAK,
    STATEMENT_CONTINUE,
    STATEMENT_BLOCK,
    STATEMENT_IF,
    STATEMENT_WHILE,
    STATEMENT_DO_WHILE,
    STATEMENT_FOR,
    STATEMENT_SWITCH,
    STATEMENT_ASSIGN,
    STATEMENT_INIT,
    STATEMENT_EXPR,
    STATEMENT_NOTE,
    STATEMENT_LABEL,
    STATEMENT_GOTO,
} StatementType;

struct Statement {
    StatementType type;
    union {
        Expression* expr;
        Declaration* decl;
        const char* label;
        struct {
            //TODO
        } ifVal;
        struct {
            //TODO
        } whileVal;
        struct {
            //TODO
        } forVal;
        struct {
            //TODO
        } switchVal;
        struct {
            //TODO
        } assign;
        struct {
            //TODO
        } init;
    };
};

#endif //LAVA_AST_H
