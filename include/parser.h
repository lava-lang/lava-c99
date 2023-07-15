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
    Parser* parser = CALLOC(1, sizeof(Parser));
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
LAVA(MSG, "Internal Error: ", __VA_ARGS__) \
printSyntaxErrorLocation(parser->lexer, start); \
exit(EXIT_FAILURE); \

Token* parserConsume(Parser* parser, TokenType type) {
    ASSERT(parser->token->type != type, "Unexpected token! expected: %s, got: %s (%s)", TOKEN_NAMES[type], TOKEN_NAMES[parser->token->type], parser->token->value);
    INFO("Consumed: %s: %s", TOKEN_NAMES[parser->token->type], parser->token->value);
    TOKENS_CONSUMED++;
    Token* prev = parser->token;
    parser->token = lexNextToken(parser->lexer);
    while (parser->token->type == TOKEN_COMMENT_LINE || parser->token->type == TOKEN_COMMENT_MULTI) {
        parser->token = lexNextToken(parser->lexer);
    }
    parser->type = parser->token->type;
    if (parser->token->type == TOKEN_UNEXPECTED) {
        ERROR("Unexpected Token! (%s)", parser->token->value);
    }
    return prev;
}

//TODO replace with bit mask
int isVarType(TokenType type) {
    return type == TOKEN_VOID || type == TOKEN_INT || type == TOKEN_I32 || type == TOKEN_I64 || type == TOKEN_FLOAT || type == TOKEN_F32 || type == TOKEN_F64 || type == TOKEN_STRING || type == TOKEN_BOOLEAN || type == TOKEN_CHAR;
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

void parseCStatement(List* nodes, Parser* parser, Scope* scope) {
    Token* statement = parser->token;
    parserConsume(parser, TOKEN_C_STATEMENT);
    listAppend(nodes, initAST(statement, AST_C_STATEMENT));
}

AST* parseFactor(Parser* parser, Scope* scope);
AST* parseExpression(Parser* parser, Scope* scope);
AST* parseAST(Parser* parser, Scope* scope, TokenType breakToken);

AST* parseFactor(Parser* parser, Scope* scope) {
    Token* token = parser->token;
    switch (parser->type) {
        //TODO replace with var bit mask
        case TOKEN_INTEGER_VALUE:
        case TOKEN_FLOAT_VALUE:
        case TOKEN_STRING_VALUE:
        case TOKEN_CHAR_VALUE:
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

void parseReturn(List* nodes, Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_RETURN);
    AST* expression = parseExpression(parser, scope);
    parserConsume(parser, TOKEN_EOS);
    listAppend(nodes, initASTReturn(expression));
}

void parseImport(List* nodes, Parser* parser, Scope* scope) {
    Token* token = parser->token;
    parserConsume(parser, TOKEN_IMPORT);
    parserConsume(parser, TOKEN_EOS);
    listAppend(nodes, initAST(token, AST_IMPORT));
}

void parseVarDefinition(List* nodes, Parser* parser, Scope* scope, AST* dataType, AST* identifier) {
    AST* expression = NULL; //TODO somehow remove use of null
    if (parser->type == TOKEN_ASSIGNMENT) {
        parserConsume(parser, TOKEN_ASSIGNMENT);
        expression = parseExpression(parser, scope);
        if (expression->token->type != ((TokenVar*) dataType->token)->validValue) {
            ERROR("%s (%s) incompatible with: %s", TOKEN_NAMES[expression->token->type], expression->token->value, TOKEN_NAMES[dataType->token->type]);
        }
    }
    listAppend(nodes, initASTVarDef(dataType, identifier, expression));
}

void parseFuncDefinition(List* nodes, Parser* parser, Scope* scope, AST* returnType, AST* identifier) {
    parserConsume(parser, TOKEN_LPAREN);
    //TODO args..
    parserConsume(parser, TOKEN_RPAREN);

    parserConsume(parser, TOKEN_LBRACE);
    AST* compound = parseAST(parser, scope, TOKEN_RBRACE);
    parserConsume(parser, TOKEN_RBRACE);
    listAppend(nodes, initASTFuncDef(returnType, identifier, compound));
}

void parseDefinition(List* nodes, Parser* parser, Scope* scope) {
    AST* dataType = NULL; AST* identifier = NULL;
    while ((isVarType(parser->type) || parser->type == TOKEN_COMMA)) {
        if (parser->type == TOKEN_COMMA) {
            parserConsume(parser, TOKEN_COMMA);
        }
        if (isVarType(parser->type)) {
            dataType = parseDataType(parser, scope);
        }
        identifier = parseIdentifier(parser, scope);
        if (parser->type == TOKEN_LPAREN) { //Parsing Function Def
            parseFuncDefinition(nodes, parser, scope, dataType, identifier);
            break; //Break out of loop, as no more work once function is constructed
        }
        parseVarDefinition(nodes, parser, scope, dataType, identifier);
        if (parser->type == TOKEN_EOS) { //Reached end of variable definitions
            parserConsume(parser, TOKEN_EOS);
            break;
        }
    }
}

void parseStructDefinition(List* nodes, Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_STRUCT_DEF);
    AST* identifier = parseIdentifier(parser, scope);
    parserConsume(parser, TOKEN_LBRACE);
    AST* members = parseAST(parser, scope, TOKEN_RBRACE);
    parserConsume(parser, TOKEN_RBRACE);
    listAppend(nodes, initASTStructDef(identifier, members));
}

AST* parseAST(Parser* parser, Scope* scope, TokenType breakToken) {
    List* nodes = listInit(sizeof(AST*));
    while (parser->type != breakToken) {
        if (isVarType(parser->type)) {
            parseDefinition(nodes, parser, scope);
        } else if (parser->type == TOKEN_STRUCT_DEF) {
            parseStructDefinition(nodes, parser, scope);
        } else if (parser->type == TOKEN_C_STATEMENT) {
            parseCStatement(nodes, parser, scope);
        } else if (parser->type == TOKEN_RETURN) {
            parseReturn(nodes, parser, scope);
        } else if (parser->type == TOKEN_IMPORT) {
            parseImport(nodes, parser, scope);
        } else {
            ERROR("Token Was Not Consumed Or Parsed! %s (%s)", TOKEN_NAMES[parser->type], parser->token->value);
        }
    }
    return initASTCompound(parser->token, nodes);
}
#endif