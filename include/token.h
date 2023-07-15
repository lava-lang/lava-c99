#ifndef LAVA_TOKEN_H
#define LAVA_TOKEN_H

#include <stdlib.h>
#include <stdbool.h>
#include "util.h"

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

    TOKEN_STRUCT_DEF,
    TOKEN_ENUM_DEF,
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

    TOKEN_DIVISION,
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
    VAR_POINTER = 1 << 0,
    VAR_ARRAY   = 1 << 1,
    VAR_TYPE    = 1 << 2,
    VAR_INT     = 1 << 3,
    VAR_FLOAT   = 1 << 4,
    VAR_STR     = 1 << 5,
    VAR_CHAR    = 1 << 6,
    VAR_BOOL    = 1 << 7,
} TokenFlag;

typedef struct Token {
    TokenType type;
    char* value;
    size_t flags;
} Token;

static Token STATIC_TOKEN_NONE = {TOKEN_NONE, "none"};

Token* tokenInitBase(Token* token, TokenType type, char* value, size_t flags) {
    token->type = type;
    if (value) {
        token->value = value;
    }
    token->flags = flags;
    return token;
}

Token* tokenInit(TokenType type, char* value, size_t flags) {
    return tokenInitBase(CALLOC(1, sizeof(Token)), type, value, flags);
}

void tokenFree(Token* token) {
    if (token->value) {
        FREE(token->value);
    }
    FREE(token);
}
#endif