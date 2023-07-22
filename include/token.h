#ifndef LAVA_TOKEN_H
#define LAVA_TOKEN_H

#include <stdlib.h>
#include <stdbool.h>
#include "util.h"
#include "region.h"
#include "strview.h"

const char* TOKEN_NAMES[] = {
        "End of File",
        "Unexpected Token",
        "None",
        "C Statement",

        "NULL",
        "Void Type",
        "Int",
        "Int 8",
        "Int 16",
        "Int 32",
        "Int 64",
        "UInt 8",
        "UInt 16",
        "UInt 32",
        "UInt 64",
        "Integer Value",
        "Float 32",
        "Float 64",
        "Float Value",
        "String Type",
        "String Value",
        "Char Type",
        "Char Value",
        "Boolean Type",
        "Boolean Value",

        "Struct",
        "Enum",
        "Flag",
        "If Statement",
        "Else Statement",
        "While Loop",
        "For Loop",
        "Return Statement",
        "Import Statement",

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
};

typedef enum TokenType {
    //Special
    TOKEN_EOF,
    TOKEN_UNEXPECTED,
    TOKEN_NONE,
    TOKEN_C_STATEMENT,

    //Basic Types
    TOKEN_NULL,
    TOKEN_VOID,
    TOKEN_INT,
    TOKEN_I8,
    TOKEN_I16,
    TOKEN_I32,
    TOKEN_I64,
    TOKEN_U8,
    TOKEN_U16,
    TOKEN_U32,
    TOKEN_U64,
    TOKEN_INTEGER_VALUE, //TODO maybe replaced with VAR_INT?
    TOKEN_F32,
    TOKEN_F64,
    TOKEN_FLOAT_VALUE,
    TOKEN_STRING,
    TOKEN_STRING_VALUE,
    TOKEN_CHAR,
    TOKEN_CHAR_VALUE,
    TOKEN_BOOLEAN,
    TOKEN_BOOLEAN_VALUE,

    TOKEN_STRUCT,
    TOKEN_ENUM,
    TOKEN_FLAG,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_IMPORT,

    TOKEN_IDENTIFIER,
    TOKEN_EOS,

    TOKEN_ASSIGNMENT,
    TOKEN_EQUALS,

    TOKEN_DIVIDE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_LESS_THAN,
    TOKEN_MORE_THAN,
    TOKEN_NOT,

    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
} TokenType;

typedef enum TokenFlag {
    DATA_TYPE   = 1 << 0,
    DATA_VALUE  = 1 << 1,
    TYPE_BINOP  = 1 << 2,

    VAR_VOID    = 1 << 9,
    VAR_POINTER = 1 << 10,
    VAR_ARRAY   = 1 << 11,
    VAR_INT     = 1 << 13,
    VAR_FLOAT   = 1 << 14,
    VAR_STR     = 1 << 15,
    VAR_CHAR    = 1 << 16,
    VAR_BOOL    = 1 << 17,
} TokenFlag;

typedef struct Token {
    TokenType type;
    StrView view;
    size_t flags;
} Token;

static Token STATIC_TOKEN_NONE = {TOKEN_NONE, "none"};

Token* tokenInit(TokenType type, StrView* view, size_t flags) {
    Token* token = RALLOC(1, sizeof(Token));
    token->type = type;
    memcpy(&token->view, view, sizeof(StrView)); //Copy view pointer, as this will be changed by the lexer
    token->flags = flags;
    return token;
}
#endif