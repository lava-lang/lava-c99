#ifndef LAVA_PARSER_H
#define LAVA_PARSER_H

#include <stdlib.h>
#include "token.h"
#include "lexer.h"
#include "structures.h"
#include "debug.h"
#include "ast.h"

static int TOKENS_CONSUMED = 0;

typedef struct Parser {
    Token* token;
    Lexer* lexer;
    TokenType type;
} Parser;

Parser* parserInit(Lexer* lexer) {
    Parser* parser = calloc(1, sizeof(Parser));
    parser->lexer = lexer;
    parser->token = lexNextToken(lexer);
    parser->type = parser->token->type;
    return parser;
}

void parserFree(Parser* parser) {
//    if (parser->token) {
//        tokenFree(parser->token);
//    }
    lexerFree(parser->lexer);
    free(parser);
}

Token* parserConsume(Parser* parser, TokenType type) {
    if (parser->token->type != type) {
        PANIC("Unexpected token! expected: %s, got: %s", TOKEN_NAMES[type], TOKEN_NAMES[parser->token->type]);
    }
    INFO("Consumed: %s (%s)\n", TOKEN_NAMES[parser->token->type], parser->token->value);
    TOKENS_CONSUMED++;
    Token* prev = parser->token;
    parser->token = lexNextToken(parser->lexer);
    parser->type = parser->token->type;
    return prev;
}

int isVarType(TokenType type) {
    return type == TOKEN_VOID || type == TOKEN_INT || type == TOKEN_I32 || type == TOKEN_I64 || type == TOKEN_FLOAT || type == TOKEN_F32 || type == TOKEN_F64 || type == TOKEN_STRING || type == TOKEN_BOOLEAN;
}

AST* parseIdentifier(Parser* parser, Scope* scope) {
    Token* identifier = parser->token;
    parserConsume(parser, TOKEN_IDENTIFIER);
    return initAST(identifier, AST_IDENTIFIER);
}

AST* parseDataType(Parser* parser, Scope* scope) {
    Token* varType = parser->token;
    parserConsume(parser, parser->type); //Type was checked by parseTokens, so this should be valid. parserConsume will panic if not.
    return initAST(varType, AST_DATA_TYPE);
}

AST* parseFactor(Parser* parser, Scope* scope);
AST* parseExpression(Parser* parser, Scope* scope);

AST* parseFactor(Parser* parser, Scope* scope) {
    Token* token = parser->token;
    switch (parser->type) {
        case TOKEN_INTEGER_VALUE:
        case TOKEN_FLOAT_VALUE:
        case TOKEN_STRING_VALUE:
        case TOKEN_BOOLEAN_VALUE:
            parserConsume(parser, parser->type);
            return initAST(token, AST_VAR_VALUE);
        case TOKEN_LPAREN:
            parserConsume(parser, TOKEN_LPAREN);
            AST* expression = parseExpression(parser, scope);
            parserConsume(parser, TOKEN_RPAREN);
            return expression;
        default:
            return parseExpression(parser, scope);
//            fprintf(stderr, "parseFactor: Unhandled Token: %s\n", TOKEN_NAMES[token->type]);
//            exit(1);
    }
}

AST* parseTerm(Parser* parser, Scope* scope) {
    AST* factor = parseFactor(parser, scope);
    return factor;
}

int isBinaryOperator(TokenType type) {
    return type == TOKEN_PLUS || type == TOKEN_MINUS || type == TOKEN_EQUALS || type == TOKEN_NOT || type == TOKEN_ASSIGNMENT || type == TOKEN_LESS_THAN || type == TOKEN_MORE_THAN;
}

AST* parseExpression(Parser* parser, Scope* scope) {
    AST* ast = parseTerm(parser, scope);

    if (isBinaryOperator(parser->type)) {
        Token* binaryOp = parser->token;
        parserConsume(parser, parser->type);

        AST* right = parseTerm(parser, scope);
        return (AST*) initASTBinaryOp(binaryOp, ast, right);
    }
    return ast;
}

ASTVarDef* parseVarDefinition(Parser* parser, Scope* scope, AST* identifier, AST* dataType) {
    AST* right = parseExpression(parser, scope);
    if (right->token->type != ((TokenVar*) dataType->token)->validValue) {
        PANIC("%s (%s) incompatible with: %s", TOKEN_NAMES[right->token->type], right->token->value, TOKEN_NAMES[dataType->token->type]);
    }
    parserConsume(parser, TOKEN_SEMI);
    return initASTVarDef(dataType, identifier, right);
}

ASTFuncDef* parseFuncDefinition(Parser* parser, Scope* scope, AST* identifier, AST* returnType) {
    return initASTFuncDef(returnType, identifier, NULL);
}

AST* parseDefinition(Parser* parser, Scope* scope) {
    AST* dataType = parseDataType(parser, scope);
    AST* identifier = parseIdentifier(parser, scope);
    if (parser->type == TOKEN_ASSIGNMENT) { //Variable definition
        parserConsume(parser, TOKEN_ASSIGNMENT);
        return (AST*) parseVarDefinition(parser, scope, identifier, dataType);
    } else if (parser->type == TOKEN_LPAREN) { //Function definition
        parserConsume(parser, TOKEN_LPAREN);
        parserConsume(parser, TOKEN_RPAREN);
        parserConsume(parser, TOKEN_LBRACE);
        parserConsume(parser, TOKEN_RBRACE);
        return (AST*) parseFuncDefinition(parser, scope, identifier, dataType);
    }
}

ASTCompound* parseAST(Parser* parser, Scope* scope) {
    List* children = listInit(sizeof(AST*));
    while (parser->type != TOKEN_EOF) {
        AST* node = NULL;
        if (isVarType(parser->type)) {
            node = parseDefinition(parser, scope);
        } else {
            PANIC("Unexpected token: %s", TOKEN_NAMES[parser->type]);
        }
        listAppend(children, node);
    }
    return initASTCompound(parser->token, children);
}
#endif