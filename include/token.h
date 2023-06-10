#ifndef LAVA_TOKEN_H
#define LAVA_TOKEN_H

#include <stdlib.h>

typedef enum TokenType {
    //Special
    TOKEN_EOF,
    TOKEN_UNEXPECTED,

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
    TOKEN_TRUE,
    TOKEN_FALSE,

    TOKEN_TYPE_DEF,
    TOKEN_ENUM_DEF,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,

    TOKEN_IDENTIFIER,
    TOKEN_SEMI,

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

Token* tokenInit(Token* token, TokenType type, char* value) {
    token->type = type;
    token->value = value;
    return token;
}

Token* tokenInitBase(TokenType type, char* value) {
    return tokenInit(calloc(1, sizeof(Token)), type, value);
}

typedef struct TokenVar {
    Token base;
    TokenType valid;
} TokenVar;

Token* tokenVarInit(TokenType type, char* value, TokenType valid) {
    TokenVar* tokenVar = calloc(1, sizeof(TokenVar));
    tokenInit((Token*) tokenVar, type, value);
    tokenVar->valid = valid;
    return (Token*) tokenVar;
}

void tokenFree(Token* token) {
    if (token->value) {
        free(token->value);
    }
    free(token);
}
#endif