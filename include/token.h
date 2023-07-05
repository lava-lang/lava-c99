#ifndef LAVA_TOKEN_H
#define LAVA_TOKEN_H

#include <stdlib.h>
#include <stdbool.h>

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
    TOKEN_I32,
    TOKEN_I64,
    TOKEN_INTEGER_VALUE,
    TOKEN_FLOAT,
    TOKEN_F32,
    TOKEN_F64,
    TOKEN_FLOAT_VALUE,
    TOKEN_STRING,
    TOKEN_STRING_VALUE,
    TOKEN_BOOLEAN,
    TOKEN_BOOLEAN_VALUE,

    TOKEN_TYPE_DEF,
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

    //Comments
    TOKEN_COMMENT_LINE,
    TOKEN_COMMENT_MULTI,

} TokenType;

typedef struct Token {
    TokenType type;
    char* value;
} Token;

Token* tokenInitBase(Token* token, TokenType type, char* value) {
    token->type = type;
    if (value) {
        token->value = value;
    }
    return token;
}

Token* tokenInit(TokenType type, char* value) {
    return tokenInitBase(calloc(1, sizeof(Token)), type, value);
}

static Token STATIC_TOKEN_NONE = {TOKEN_NONE, "none"};

typedef struct TokenVar {
    Token base;
    TokenType validValue;
    bool isPointer;
} TokenVar;

Token* tokenVarInit(TokenType type, char* value, TokenType validValue, bool isPointer) {
    TokenVar* tokenVar = calloc(1, sizeof(TokenVar));
    tokenInitBase((Token*) tokenVar, type, value);
    tokenVar->validValue = validValue;
    tokenVar->isPointer = isPointer;
    return (Token*) tokenVar;
}

void tokenFree(Token* token) {
    if (token->value) {
        free(token->value);
    }
    free(token);
}
#endif