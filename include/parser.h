#ifndef LAVA_PARSER_H
#define LAVA_PARSER_H

#include <stdlib.h>
#include "token.h"
#include "lexer.h"
#include "ast.h"

static int TOKENS_CONSUMED = 0;

typedef struct Parser {
    Token* token;
    Lexer* lexer;
    TokenType type;
} Parser;

Parser* parserInit(Lexer* lexer) {
    Parser* parser = MALLOC(sizeof(Parser));
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

#undef ERROR //Redefine ERROR now that Lexing has finished
#define ERROR(MSG, ...) \
size_t start = findStartOfErrorSnippet(parser->lexer); \
printf("\n%s:%zu:%zu: ", parser->lexer->filepath, parser->lexer->line, parser->lexer->pos - parser->lexer->col); \
LAVA(MSG, "Lava Error: ", __VA_ARGS__) \
printSyntaxErrorLocation(parser->lexer, start); \
exit(EXIT_FAILURE); \

Token* parserConsume(Parser* parser, TokenType type) {
    if (parser->token->type != type) {
        PANIC("Unexpected token! expected: %s, got: %s", TOKEN_NAMES[type], TOKEN_NAMES[parser->token->type]);
    }
    INFO("Consumed: %s: %s", TOKEN_NAMES[parser->token->type], parser->token->value);
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

AST* parseCStatement(Parser* parser, Scope* scope) {
    Token* statement = parser->token;
    parserConsume(parser, TOKEN_C_STATEMENT);
    return initAST(statement, AST_C_STATEMENT);
}

AST* parseFactor(Parser* parser, Scope* scope);
AST* parseExpression(Parser* parser, Scope* scope);
AST* parseAST(Parser* parser, Scope* scope, TokenType breakToken);

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
        return initASTBinaryOp(binaryOp, ast, right);
    }
    return ast;
}

AST* parseVarDefinition(Parser* parser, Scope* scope, AST* identifier, AST* dataType) {
    AST* right = parseExpression(parser, scope);
    if (right->token->type != ((TokenVar*) dataType->token)->validValue) {
        ERROR("%s (%s) incompatible with: %s", TOKEN_NAMES[right->token->type], right->token->value, TOKEN_NAMES[dataType->token->type]);
    }
    parserConsume(parser, TOKEN_EOS);
    return initASTVarDef(dataType, identifier, right);
}

AST* parseFuncDefinition(Parser* parser, Scope* scope, AST* identifier, AST* returnType) {
    parserConsume(parser, TOKEN_LPAREN);
    //TODO args..
    parserConsume(parser, TOKEN_RPAREN);

    parserConsume(parser, TOKEN_LBRACE);
    AST* compound = parseAST(parser, scope, TOKEN_RBRACE);
    parserConsume(parser, TOKEN_RBRACE);
    return initASTFuncDef(returnType, identifier, compound);
}

AST* parseDefinition(Parser* parser, Scope* scope) {
    AST* dataType = parseDataType(parser, scope);
    AST* identifier = parseIdentifier(parser, scope);
    if (parser->type == TOKEN_ASSIGNMENT) { //Variable definition
        parserConsume(parser, TOKEN_ASSIGNMENT);
        return parseVarDefinition(parser, scope, identifier, dataType);
    } else if (parser->type == TOKEN_LPAREN) { //Function definition
        return parseFuncDefinition(parser, scope, identifier, dataType);
    }
}

AST* parseReturn(Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_RETURN);
    AST* expression = parseExpression(parser, scope);
    parserConsume(parser, TOKEN_EOS);
    return initASTReturn(expression);
}

AST* parseImport(Parser* parser, Scope* scope) {
    Token* token = parser->token;
    parserConsume(parser, TOKEN_IMPORT);
    parserConsume(parser, TOKEN_EOS);
    return initAST(token, AST_IMPORT);
}

AST* parseAST(Parser* parser, Scope* scope, TokenType breakToken) {
    List* children = listInit(sizeof(AST*));
    while (parser->type != breakToken) {
        AST* node = NULL;
        if (isVarType(parser->type)) {
            node = parseDefinition(parser, scope);
        } else if (parser->type == TOKEN_COMMENT_LINE) {
            parserConsume(parser, TOKEN_COMMENT_LINE);
            continue; //NOOP
        } else if (parser->type == TOKEN_C_STATEMENT) {
            node = parseCStatement(parser, scope);
        } else if (parser->type == TOKEN_RETURN) {
            node = parseReturn(parser, scope);
        } else if (parser->type == TOKEN_IMPORT) {
            node = parseImport(parser, scope);
        }

        else {
            PANIC("Could not parse AST for: %s", TOKEN_NAMES[parser->type]);
        }
        listAppend(children, node);
    }
    return initASTCompound(parser->token, children);
}
#endif