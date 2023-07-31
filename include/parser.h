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
    Parser* parser = RALLOC(1, sizeof(Parser));
    parser->lexer = lexer;
    parser->token = lexNextToken(lexer);
    parser->type = parser->token->type;
    return parser;
}

#undef ERROR //Redefine ERROR now that Lexing has finished
#define ERROR(MSG, ...) \
size_t start = findStartOfErrorSnippet(parser->lexer); \
printf("\n%s:%zu:%zu: ", parser->lexer->filepath, parser->lexer->line, parser->lexer->pos - parser->lexer->col); \
LAVA(MSG, "Internal Error: ", __VA_ARGS__) \
printSyntaxErrorLocation(parser->lexer, start); \
exit(EXIT_FAILURE); \

Token* parserConsume(Parser* parser, TokenType type) {
    ASSERT(parser->token->type != type, "Unexpected token! expected: %s, got: %s (%s)", TOKEN_NAMES[type], TOKEN_NAMES[parser->token->type], viewToStr(&parser->token->view));
    INFO("Consumed: %s: %s", TOKEN_NAMES[parser->token->type], viewToStr(&parser->token->view));
    TOKENS_CONSUMED++;
    Token* prev = parser->token;
    parser->token = lexNextToken(parser->lexer);
    parser->type = parser->token->type;
    if (parser->token->type == TOKEN_UNEXPECTED) {
        ERROR("Unexpected Token! (%s)", viewToStr(&parser->token->view));
    }
    return prev;
}

//TODO, pass the return of parserConsume directly
AST* parseIdentifier(Parser* parser, Scope* scope) {
    Token* identifier = parser->token;
    parserConsume(parser, TOKEN_ID);
    return valueAST(AST_ID, 0, token, identifier);
}

AST* parseDataType(Parser* parser, Scope* scope) {
    Token* varType = parser->token;
    parserConsume(parser, parser->type); //Type was checked by parseTokens, so this should be valid. parserConsume will panic if not.
    return valueAST(AST_TYPE, 0, token, varType);
}

void parseCStatement(DynArray* nodes, Parser* parser, Scope* scope) {
    Token* statement = parser->token;
    parserConsume(parser, TOKEN_C_STATEMENT);
    arrayAppend(nodes, valueAST(AST_C, 0, token, statement));
}

AST* parseFactor(Parser* parser, Scope* scope);
AST* parseExpression(Parser* parser, Scope* scope);
void parseDefinition(DynArray* nodes, Parser* parser, Scope* scope);
AST* parseAST(Parser* parser, Scope* scope, TokenType breakToken);

AST* parseFactor(Parser* parser, Scope* scope) {
    Token* token = parser->token;
    if (parser->token->flags & DATA_VALUE) {
        parserConsume(parser, parser->type);
        return valueAST(AST_VALUE, 0, token, token);
    } else if (parser->type == TOKEN_LPAREN) {
        parserConsume(parser, TOKEN_LPAREN);
        AST* expression = parseExpression(parser, scope);
        parserConsume(parser, TOKEN_RPAREN);
        return expression;
    } else if (parser->type == TOKEN_LBRACKET) {
        //TODO parse array
    } else {
        return parseExpression(parser, scope);
    }
}

AST* parseTerm(Parser* parser, Scope* scope) {
    AST* factor = parseFactor(parser, scope);
    while (parser->type == TOKEN_DIVIDE || parser->type == TOKEN_MULTIPLY) {
        Token* token = parser->token;
        parserConsume(parser, parser->type);
        factor = structAST(AST_BINOP, 0, binop, factor, token, parseFactor(parser, scope));
    }
    return factor;
}

AST* parseExpression(Parser* parser, Scope* scope) {
    AST* ast = parseTerm(parser, scope);
    if (parser->token->flags & TYPE_BINOP) {
        Token* token = parser->token;
        parserConsume(parser, parser->type);
        return structAST(AST_BINOP, 0, binop, ast, token, parseTerm(parser, scope));
    }
    return ast;
}

void parseReturn(DynArray* nodes, Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_RETURN);
    arrayAppend(nodes, valueAST(AST_RETURN, 0, expression, parseExpression(parser, scope)));
    parserConsume(parser, TOKEN_EOS);
}

void parseImport(DynArray* nodes, Parser* parser, Scope* scope) {
    arrayAppend(nodes, valueAST(AST_IMPORT, 0, token, parser->token));
    parserConsume(parser, TOKEN_IMPORT);
    parserConsume(parser, TOKEN_EOS);
}

bool isValueCompatible(AST* dataType, AST* expression) {
    return true;
    if (dataType->token->flags & VAR_INT) {
        return expression->token->type == TOKEN_INTEGER_VALUE;
    } else if (dataType->token->flags & VAR_FLOAT) {
        return expression->token->type == TOKEN_FLOAT_VALUE;
    } else if (dataType->token->flags & VAR_STR) {
        return expression->token->type == TOKEN_STRING_VALUE;
    } else if (dataType->token->flags & VAR_CHAR) {
        return expression->token->type == TOKEN_CHAR_VALUE;
    } else if (dataType->token->flags & VAR_BOOL) {
        return expression->token->type == TOKEN_BOOLEAN_VALUE;
    } else {
        return false;
    }
}

void parseVarDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* dataType, AST* identifier) {
    AST* expression = NULL; //TODO somehow remove use of null
    if (parser->type == TOKEN_ASSIGNMENT) {
        parserConsume(parser, TOKEN_ASSIGNMENT);
        expression = parseExpression(parser, scope);
        if (!isValueCompatible(dataType, expression)) {
            ERROR("%s (%s) incompatible with: %s", TOKEN_NAMES[expression->token->type], viewToStr(&expression->token->view), TOKEN_NAMES[dataType->token->type]);
        }
    }
    arrayAppend(nodes, structAST(AST_VAR, 0, varDef, dataType, identifier, expression));
}

void parseFuncDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* returnType, AST* identifier) {
    parserConsume(parser, TOKEN_LPAREN);
    DynArray* nodesArgs = arrayInit(sizeof(AST *));
    parseDefinition(nodesArgs, parser, scope);
    for (int i = 0; i < nodesArgs->len; ++i) {
        ((AST*) nodesArgs->elements[i])->flags |= ARGUMENT;
    }
    AST* args = valueAST(AST_COMP, 0, array, nodesArgs);
    parserConsume(parser, TOKEN_RPAREN);
    parserConsume(parser, TOKEN_LBRACE);
    AST* body = parseAST(parser, scope, TOKEN_RBRACE);
    parserConsume(parser, TOKEN_RBRACE);
    arrayAppend(nodes, structAST(AST_FUNC, 0, funcDef, returnType, identifier, args, body));
}

void parseDefinition(DynArray* nodes, Parser* parser, Scope* scope) {
    AST* dataType = NULL;
    while (parser->token->flags & DATA_TYPE || parser->type == TOKEN_ID) {
        if (parser->type == TOKEN_ID) {
            ASSERT(dataType == NULL, "Comma delimited var def did not start with valid dataType! found (%s)", TOKEN_NAMES[parser->type])
            //TODO: alloc new token as copy of dataType->token?
            dataType = valueAST(AST_TYPE, 0, token, dataType->token);
        } else if (parser->token->flags & DATA_TYPE) {
            dataType = parseDataType(parser, scope);
        }
        //First conditional chain, testing to detect func pointer, function def, or standard variable
        /*if (parser->type == TOKEN_COLON) { //Parsing function pointer
            parserConsume(parser, TOKEN_COLON);
            parserConsume(parser, TOKEN_LPAREN);
            DynArray* argumentArray = arrayInit(sizeof(AST*));
            while (parser->type != TOKEN_RPAREN) { //Consume function argument types
                arrayAppend(argumentArray, parseDataType(parser, scope));
                if (parser->type == TOKEN_COMMA) {
                    parserConsume(parser, TOKEN_COMMA);
                }
            }
            parserConsume(parser, TOKEN_RPAREN);
            AST* identifier = parseIdentifier(parser, scope); //Construct function pointer AST
            AST* argumentTypes = valueAST(AST_COMP, 0, array, argumentArray);
            arrayAppend(nodes, structAST(AST_FUNC_VAR, 0, funcDef, dataType, identifier, argumentTypes, NULL));
            //TODO fix hack. this stops function pointers being supported by comma seperated definition without explicit data types.
            // this should be fixed at some point. Maybe function pointers need their own function instead of being inside parseDefinition?
            dataType = NULL;
        } else */if (parser->type == TOKEN_ID) { //If we get here, we must be parsing a function or variable
            AST* identifier = parseIdentifier(parser, scope);
            if (parser->type == TOKEN_LPAREN) { //Parsing Function Def
                parseFuncDefinition(nodes, parser, scope, dataType, identifier);
                break; //Break out of loop, as no more work once function is constructed
            }
            parseVarDefinition(nodes, parser, scope, dataType, identifier); //Finally, it must be a standard variable
        }
        //Final conditional chain, regardless of the outcome of the previous chain, we want to handle COMMA and EOS
        if (parser->type == TOKEN_EOS) { //Reached end of variable definitions
            parserConsume(parser, TOKEN_EOS);
            break;
        } else if (parser->type == TOKEN_COMMA) { //More variable definitions to come, continue
            parserConsume(parser, TOKEN_COMMA);
            continue;
        }
    }
}

void parseStructDefinition(DynArray* nodes, Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_STRUCT);
    ASTFlag flags = 0;
//    if (parser->type == TOKEN_PACKED) { //Apply GCC data packing
//        parserConsume(parser, TOKEN_PACKED);
//        flags |= PACKED_DATA;
//    }
    AST* id = parseIdentifier(parser, scope);
    parserConsume(parser, TOKEN_LBRACE);
    arrayAppend(nodes, structAST(AST_STRUCT, flags, structDef, id, parseAST(parser, scope, TOKEN_RBRACE)));
    parserConsume(parser, TOKEN_RBRACE);
}

void parseEnumDefinition(DynArray* nodes, Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_ENUM);
    ASTFlag flags = 0;
    if (parser->type == TOKEN_FLAG) { //Bitmask enum
        parserConsume(parser, TOKEN_FLAG);
        flags |= ENUM_FLAG;
    }
//    if (parser->type == TOKEN_PACKED) { //Apply GCC data packing
//        parserConsume(parser, TOKEN_PACKED);
//        flags |= PACKED_DATA;
//    }
    AST* identifier = parser->type == TOKEN_ID ? parseIdentifier(parser, scope) : NULL;
    AST* dataType = NULL;
//    if (parser->token->flags & DATA_TYPE) { //Enum has explicit value size
//        if ((parser->token->flags & VAR_INT) == 0) { //Only allow integer value sizes
//            ERROR("Enum constant type must be integer! %s (%s)", TOKEN_NAMES[parser->type], viewToStr(&parser->token->view))
//        }
//        dataType = parseDataType(parser, scope);
//    }
    parserConsume(parser, TOKEN_LBRACE);
    DynArray* constantArray = arrayInit(sizeof(AST*));
    size_t constantValue = 0; //Initial enum constant value
    while (parser->type == TOKEN_ID) {
        AST* constant = parseIdentifier(parser, scope);
        if (parser->type == TOKEN_ASSIGNMENT) { //Assigning value to enum constant
            parserConsume(parser, TOKEN_ASSIGNMENT);
            AST* expression = parseExpression(parser, scope);
            arrayAppend(nodes, structAST(AST_ASSIGN, 0, assign, constant, expression));
        } else { //No assigment, so default generated value
            if (flags & ENUM_FLAG) { //If it's a flag value, auto increment ^2
                AST* integer = valueAST(AST_INTEGER, 0, value, 1 << constantValue);
                arrayAppend(constantArray, structAST(AST_ASSIGN, 0, assign, constant, integer));
            } else { //Normal enum constant, no assignment or flag
                arrayAppend(constantArray, constant);
            }
        }
        if (parser->type == TOKEN_COMMA) parserConsume(parser, TOKEN_COMMA);
        if (parser->type == TOKEN_RBRACE) break;
        constantValue++;
    }
    parserConsume(parser, TOKEN_RBRACE);
    AST* constants = valueAST(AST_COMP, 0, array, constantArray); //TODO move this to parseAST? like struct members?
    arrayAppend(nodes, structAST(AST_ENUM, flags, enumDef, identifier, dataType, constants));
}

void parseUnionDefinition(DynArray* nodes, Parser* parser, Scope* scope) {
    parserConsume(parser, TOKEN_UNION);
    AST* identifier = parser->type == TOKEN_ID ? identifier = parseIdentifier(parser, scope) : NULL;
    parserConsume(parser, TOKEN_LBRACE);
    DynArray* memberArray = arrayInit(sizeof(AST*));
    //TODO
    parserConsume(parser, TOKEN_RBRACE);
    AST* members = valueAST(AST_COMP, 0, array, memberArray);
    arrayAppend(nodes, structAST(AST_UNION, 0, unionDef, identifier, members));
}

AST* parseAST(Parser* parser, Scope* scope, TokenType breakToken) {
    DynArray* nodes = arrayInit(sizeof(AST*));
    while (parser->type != breakToken) {
        if (parser->token->flags & DATA_TYPE) {
            parseDefinition(nodes, parser, scope);
        } else if (parser->type == TOKEN_STRUCT) {
            parseStructDefinition(nodes, parser, scope);
        } else if (parser->type == TOKEN_ENUM) {
            parseEnumDefinition(nodes, parser, scope);
        }/* else if (parser->type == TOKEN_UNION) {
            parseUnionDefinition(nodes, parser, scope);
        }*/ else if (parser->type == TOKEN_C_STATEMENT) {
            parseCStatement(nodes, parser, scope);
        } else if (parser->type == TOKEN_RETURN) {
            parseReturn(nodes, parser, scope);
        } else if (parser->type == TOKEN_IMPORT) {
            parseImport(nodes, parser, scope);
        } else {
            ERROR("Token Was Not Consumed Or Parsed! %s (%s)", TOKEN_NAMES[parser->type], viewToStr(&parser->token->view));
        }
    }
    return valueAST(AST_COMP, 0, array, nodes);
}
#endif