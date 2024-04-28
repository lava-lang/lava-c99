#ifndef LAVA_PARSER_H
#define LAVA_PARSER_H

#include <stdlib.h>
#include "token.h"
#include "lexer.h"
#include "ast.h"
#include "hashmap.h"

static int TOKENS_CONSUMED = 0;
static int STACK_MAX = 255;
static char empty[1] = {'\0'};

typedef enum StackType {
    ST_VAR,
    ST_VAR_STRUCT,
    ST_FUNC,
    ST_FUNC_STRUCT,
} StackType;

typedef struct StackEntry {
    StrView view;
    StackType type;
    bool pointer; //TODO hack..
} StackEntry;

typedef struct Scope {
    StackEntry stack[255];
    hash_table* defs;
    //TODO table for registered types
    int top;
} Scope;

Scope* scopeInit() {
    Scope* scope = CALLOC(1, sizeof(Scope));
    scope->defs = init_hash_table(100);
    scope->top = -1;
    return scope;
}

void scopeFree(Scope* scope) {
    FREE(scope);
}

//Check if the stack is empty
int stackEmpty(Scope* scope){
    if (scope->top == -1) {
        return 1;
    } else {
        return 0;
    }
}

//Check if the stack is full
int stackFull(Scope* scope) {
    if (scope->top == STACK_MAX) {
        return 1;
    } else {
        return 0;
    }
}

//Function to return the topmost element in the stack */
StackEntry stackPeek(Scope* scope) {
    return scope->stack[scope->top];
}

//Function to delete from the stack
StackEntry stackPop(Scope* scope) {
    StackEntry data;
    if(!stackEmpty(scope)) {
        data = scope->stack[scope->top];
        scope->top = scope->top - 1;
        return data;
    } else {
        printf("Could not retrieve data, Stack is empty.\n");
    }
}

//Function to insert into the stack
StackEntry stackPush(Scope* scope, StrView* data, StackType type, bool pointer) {
    if(!stackFull(scope)) {
        scope->top = scope->top + 1;
        memcpy(&scope->stack[scope->top].view, data, sizeof(StrView)); //Copy view pointer, as this will be changed by the lexer
        scope->stack[scope->top].type = type;
        scope->stack[scope->top].pointer = pointer;
    } else {
        printf("Could not insert data, Stack is full.\n");
    }
}

int getPosForIden(Scope* scope, StrView* data) {
    for (int i = 0; i < scope->top + 1; ++i) {
        if (viewViewCmp(&scope->stack[i].view, data) == true) {
            return i;
        }
    }
    return -1;
}

bool identifierExists(Scope* scope, StrView* view) {
    return getPosForIden(scope, view) == -1 ? false : true;
}

StackType getSTForIden(Scope* scope, StrView* view) {
    int pos = getPosForIden(scope, view);
    if (pos != -1) {
        return scope->stack[pos].type;
    }
    return -1;
}

bool getIsPointerForIden(Scope* scope, StrView* view) {
    int pos = getPosForIden(scope, view);
    if (pos != -1) {
        return scope->stack[pos].pointer;
    }
    return -1;
}

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
    ASSERT(parser->token->type != type, "Unexpected token! expected: %s, got: %s (%s) - %llu", TOKEN_NAMES[type], TOKEN_NAMES[parser->token->type], viewToStr(&parser->token->view), parser->lexer->line);
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
AST* parseIdentifier(Parser* parser, Scope* scope, AST* parent) {
    Token* identifier = parser->token;
    parserConsume(parser, TOKEN_ID);
    return basicAST(AST_ID, 0, identifier);
}

AST* parseDataType(Parser* parser, Scope* scope, AST* parent) {
    Token* varType = parser->token;
    parserConsume(parser, parser->type); //Type was checked by parseTokens, so this should be valid. parserConsume will panic if not.
    int flags = 0;
    if (varType->type == TOKEN_STRING) {
        flags |= POINTER_TYPE;
    }
    if (parser->type == TOKEN_STAR) {
        parserConsume(parser, TOKEN_STAR);
        flags |= POINTER_TYPE;
    }
    return basicAST(AST_TYPE, flags, varType);
}

void parseCStatement(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    Token* statement = parser->token;
    parserConsume(parser, TOKEN_C_STATEMENT);
    arrayAppend(nodes, basicAST(AST_C, 0, statement));
}

AST* parseFactor(Parser* parser, Scope* scope, AST* parent);
AST* parseExpression(Parser* parser, Scope* scope, AST* parent);
void parseDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* parent);
ASTComp* parseAST(Parser* parser, Scope* scope, TokenType breakToken, AST* parent);

AST* parseFunctionCall(Parser* parser, Scope* scope, AST* parent, AST* identifier, ASTStructDef* structDef, char* prefix) {
    parserConsume(parser, TOKEN_LPAREN);
    DynArray* expressionArray = arrayInit(sizeof(AST*));
    if (structDef != NULL) { //append struct members to expression list
        for (int i = 0; i < structDef->members->array->len; ++i) {
            AST* member = (AST*) structDef->members->array->elements[i];
            if (member->type == AST_VAR) {
                ASTVarDef* var = (ASTVarDef*) member;
                arrayAppend(expressionArray, structAST(AST_STRUCT_MEMBER_REF, 0, ASTStructMemberRef, parent, var->identifier));
            }
        }
    }
    while (parser->type != TOKEN_RPAREN) { //Consume function argument expressions
        arrayAppend(expressionArray, parseExpression(parser, scope, parent));
        if (parser->type == TOKEN_COMMA) {
            parserConsume(parser, TOKEN_COMMA);
        }
    }
    parserConsume(parser, TOKEN_RPAREN);
    ASTComp* expressions = structAST(AST_COMP, 0, ASTComp, expressionArray);
    return (AST*) structAST(AST_FUNC_CALL, 0, ASTFuncCall, identifier, expressions, prefix);
}

AST* parseFactor(Parser* parser, Scope* scope, AST* parent) {
    Token* token = parser->token;
    if (parser->token->flags & DATA_VALUE) {
        parserConsume(parser, parser->type);
        return basicAST(AST_VALUE, 0, token);
    } else if (parser->token->flags & TYPE_UNARY) {
        parserConsume(parser, parser->type);
        AST* expression = parseExpression(parser, scope, parent);
        return (AST*) structAST(AST_UNARY, UNARY_RIGHT, ASTUnary, expression, token);
    } else if (parser->type == TOKEN_LPAREN) {
        parserConsume(parser, TOKEN_LPAREN);
        AST* expression = (AST*) structAST(AST_EXPR, 0, ASTExpr, parseExpression(parser, scope, parent));
        parserConsume(parser, TOKEN_RPAREN);
        return expression;
    } else if (parser->type == TOKEN_LBRACKET) {
        //TODO parse array
    } else if (parser->type == TOKEN_RBRACE) {
        parserConsume(parser, TOKEN_LBRACE);
        parserConsume(parser, TOKEN_RBRACE);
    } else if (parser->type == TOKEN_ID) {
        //TODO validate if this is the right place for the existing iden check
        int stType = getSTForIden(scope, &parser->token->view);
        if (stType == -1) {
            ERROR("Unknown Identifier! (%s)", viewToStr(&parser->token->view));
        } else if (stType == ST_VAR) {
            return parseIdentifier(parser, scope, parent);;
        } else if (stType == ST_VAR_STRUCT) {
            AST* identifier = parseIdentifier(parser, scope, parent);
            if (parser->type == TOKEN_DOT || parser->type == TOKEN_PTR_ACCESS) { //We're doing a struct instance function call
                bool pointer = getIsPointerForIden(scope, &identifier->token->view);
                if (pointer == true) {
                    parserConsume(parser, TOKEN_PTR_ACCESS);
                } else {
                    parserConsume(parser, TOKEN_DOT);
                }
                //Check of we're trying to access a struct member
                hash_table_entry out = {0};
                int result = find(scope->defs, viewToStr(&identifier->token->view), &out);
                if (result == 0) {
                    ASTStructDef* structDef = (ASTStructDef*) out.value;
                    for (int i = 0; i < structDef->members->array->len; ++i) {
                        if (((AST*) structDef->members->array->elements[i])->type == AST_VAR) {
                            ASTVarDef* varDef = (ASTVarDef*) structDef->members->array->elements[i];
                            if (viewViewCmp(&varDef->identifier->token->view, &parser->token->view) == true) {
                                //struct member access is a defined member
                                AST* member = parseIdentifier(parser, scope, parent);
                                if (parser->type == TOKEN_EOS) {
                                    parserConsume(parser, TOKEN_EOS);
                                }
                                return (AST*) structAST(AST_STRUCT_MEMBER_REF, 0, ASTStructMemberRef, identifier, member);
                            }
                        }
                    }
                }
                //Check if we're trying to access struct function
                AST* funcIden = parseIdentifier(parser, scope, parent);
                result = find(scope->defs, viewToStr(&funcIden->token->view), &out);
                if (result == 0) {
                    ASTStructDef* structDef = (ASTStructDef*) out.value;
                    //We also pass the variable identifier as parent for the function call, so this func as access to it
                    return parseFunctionCall(parser, scope, identifier, funcIden, structDef, viewToStr(&structDef->identifier->token->view));
                }
                //Tried to call function on struct that does not exist
                ERROR("Unknown Struct Function! (%s)", viewToStr(&funcIden->token->view))
            } else {
                return identifier;
            }
        } else if (stType == ST_FUNC) {
            return parseFunctionCall(parser, scope, parent, parseIdentifier(parser, scope, parent), NULL, NULL);
        }
    } else {
        return parseExpression(parser, scope, parent);
    }
}

AST* parseTerm(Parser* parser, Scope* scope, AST* parent) {
    AST* factor = parseFactor(parser, scope, parent);
    while (parser->type == TOKEN_DIVIDE || parser->type == TOKEN_STAR || parser->type == TOKEN_EQUALITY ||
           parser->type == TOKEN_LESS_THAN || parser->type == TOKEN_LESS_THAN_OR_EQ ||
           parser->type == TOKEN_MORE_THAN || parser->type == TOKEN_MORE_THAN_OR_EQ ||
           parser->type == TOKEN_MODULUS || parser->type == TOKEN_NOT_EQUAL || parser->type == TOKEN_AND) {
        Token* token = parser->token;
        parserConsume(parser, parser->type);
        AST* right = parseFactor(parser, scope, parent);
        factor = (AST*) structAST(AST_BINOP, 0, ASTBinop, factor, token, right);
    }
    return factor;
}

AST* parseExpression(Parser* parser, Scope* scope, AST* parent) {
    AST* left = parseTerm(parser, scope, parent);
    while (parser->type == TOKEN_PLUS || parser->type == TOKEN_MINUS) {
        Token* token = parser->token;
        parserConsume(parser, parser->type);
        AST* binop = (AST*) structAST(AST_BINOP, 0, ASTBinop, left, token, parseExpression(parser, scope, parent));
        return binop;
    }
    return left;
}

void parseReturn(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_RETURN);
    AST* expression = parseExpression(parser, scope, parent);
    arrayAppend(nodes, structAST(AST_RETURN, 0, ASTExpr, expression));
    parserConsume(parser, TOKEN_EOS);
}

void parseImport(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    arrayAppend(nodes, basicAST(AST_IMPORT, 0, parser->token));
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

AST* parseIdentifierOrType(Parser* parser, Scope* scope, AST* parent) {
    AST* identifier = parseIdentifier(parser, scope, parent);
    hash_table_entry out = {0};
    int result = find(scope->defs, viewToStr(&identifier->token->view), &out);
    if (result == 0) {
        //We can cast this to AST, as we know this is the only type inserted into the table
        return (AST*) out.value;
    }
    return identifier;
}

AST* parseStructInit(DynArray* nodes, Parser* parser, Scope* scope, AST* type, AST* identifier, AST* parent) {
    parserConsume(parser, TOKEN_LBRACE);
    DynArray* expressionArray = arrayInit(sizeof(AST*));
    while (parser->type != TOKEN_RBRACE) { //Consume function argument expressions
        arrayAppend(expressionArray, parseExpression(parser, scope, parent));
        if (parser->type == TOKEN_COMMA) {
            parserConsume(parser, TOKEN_COMMA);
        }
    }
    ASTStructDef* structDef = (ASTStructDef*) type;
    //TODO store this info in the struct AST
    size_t structVarCount = 0;
    for (int i = 0; i < structDef->members->array->len; ++i) {
        if (((AST*) structDef->members->array->elements[i])->type == AST_VAR) {
            structVarCount++;
        }
    }
    if (expressionArray->len != structVarCount) {
        ERROR("Not enough init values! expected %llu, got %llu", structVarCount, expressionArray->len);
    }

    //Push this type to user defined
    insert(scope->defs, viewToStr(&identifier->token->view), structDef);

    parserConsume(parser, TOKEN_RBRACE);
    parserConsume(parser, TOKEN_EOS);
    ASTComp* expressions = structAST(AST_COMP, 0, ASTComp, expressionArray);
    arrayAppend(nodes, structAST(AST_STRUCT_INIT, 0, ASTStructInit, identifier, structDef, expressions));
}

void parseVarDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* dataType, AST* identifier, AST* parent) {
    //because this is null, assume it must be a user defined type
    if (identifier == NULL) {
        identifier = parseIdentifier(parser, scope, parent);
        if (dataType == NULL) {
            ERROR("Tried to ref user type, but not in Def table! (%s)", viewToStr(&dataType->token->view));
        }
    }

    if (identifierExists(scope, &identifier->token->view) == true) {
        ERROR("Duplicate Identifier! (%s)", viewToStr(&identifier->token->view));
    }

    AST* expression = NULL; //TODO somehow remove use of null
    if (parser->type == TOKEN_ASSIGNMENT) {
        parserConsume(parser, TOKEN_ASSIGNMENT);
        if (parser->type == TOKEN_LBRACE) { //Struct def
            stackPush(scope, &identifier->token->view, ST_VAR_STRUCT, false);
            parseStructInit(nodes, parser, scope, dataType, identifier, parent);
            return; //Avoid having a varDef node added, StructInit will handle those.
        } else { //Otherwise, assume this is standard expression
            stackPush(scope, &identifier->token->view, ST_VAR, false);
            expression = parseExpression(parser, scope, parent);
            if (!isValueCompatible(dataType, expression)) {
                ERROR("%s (%s) incompatible with: %s", TOKEN_NAMES[expression->token->type], viewToStr(&expression->token->view), TOKEN_NAMES[dataType->token->type]);
            }
        }
    } else { //If we're not assigning, then this must be a function arg.
        stackPush(scope, &identifier->token->view, ST_VAR, false);
    }
    arrayAppend(nodes, structAST(AST_VAR, 0, ASTVarDef, dataType, identifier, expression));
}

void parseFuncDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* returnType, AST* identifier, AST* parent) {
    int flags = parent->type == AST_STRUCT ? STRUCT_FUNC : 0;

    if (identifierExists(scope, &identifier->token->view) == true) {
        ERROR("Duplicate Identifier! (%s)", viewToStr(&identifier->token->view));
    }
    char* structIden = NULL;
    DynArray* nodesArgs = arrayInit(sizeof(AST *));
    if (parent->type == AST_STRUCT) {
        stackPush(scope, &identifier->token->view, ST_FUNC_STRUCT, false);
        ASTStructDef* astStructDef = (ASTStructDef*) parent;
        insert(scope->defs, viewToStr(&identifier->token->view), astStructDef);
        structIden = viewToStr(&astStructDef->identifier->token->view);

        //Add struct member vars to member function argument list
        for (int i = 0; i < nodes->len; ++i) {
            if (((AST*) nodes->elements[i])->type == AST_VAR) {
                ASTVarDef* member = (ASTVarDef*) nodes->elements[i];
                //TODO avoid copying these AST nodes. referencing the member nodes causes issues with Cgen
                ASTVarDef* copy = RALLOC(1, sizeof(ASTVarDef));
                memcpy(copy, member, sizeof(ASTVarDef));
                arrayAppend(nodesArgs, copy);
            }
        }
    } else {
        stackPush(scope, &identifier->token->view, ST_FUNC, false);
    }

    int stackTop = scope->top;
    parserConsume(parser, TOKEN_LPAREN);
    parseDefinition(nodesArgs, parser, scope, parent);
    for (int i = 0; i < nodesArgs->len; ++i) {
        ((AST*) nodesArgs->elements[i])->flags |= ARGUMENT;
        //stackPush(scope, &((AST*) nodesArgs)->token->view, ST_VAR);
    }
    ASTComp* args = structAST(AST_COMP, 0, ASTComp, nodesArgs);
    parserConsume(parser, TOKEN_RPAREN);
    parserConsume(parser, TOKEN_LBRACE);
    ASTComp* body = parseAST(parser, scope, TOKEN_RBRACE, parent);
    parserConsume(parser, TOKEN_RBRACE);
    scope->top = stackTop;
    arrayAppend(nodes, structAST(AST_FUNC, flags, ASTFuncDef, returnType, identifier, args, body, structIden));
}

void parseDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    AST* dataType = NULL;
    while (parser->token->flags & DATA_TYPE || parser->type == TOKEN_ID) {
        if (parser->type == TOKEN_ID) {
            hash_table_entry out = {0};
            int result = find(scope->defs, viewToStr(&parser->token->view), &out);
            //Use this identifier as data type, as it must be a user defined type
            if (result == 0) {
                AST* identifier = parseIdentifier(parser, scope, parent);
                /*if (result != 0) { //User defined type does not exist
                    ERROR("Type %s does not exist!", viewToStr(&identifier->token->view))
                }*/
                dataType = (AST*) basicAST(AST_TYPE, 0, identifier->token);
                if (parser->type == TOKEN_STAR) {
                    parserConsume(parser, TOKEN_STAR);
                    dataType->flags |= POINTER_TYPE;
                }
            } else {
                //TODO: alloc new token as copy of dataType->token?
                Token* token = dataType->token;
                dataType = basicAST(AST_TYPE, dataType->flags, dataType->token);
            }
        } else if (parser->token->flags & DATA_TYPE) {
            dataType = parseDataType(parser, scope, parent);
        }
        //First conditional chain, testing to detect func pointer, function def, or standard variable
        if (parser->type == TOKEN_COLON) { //Parsing function pointer
            parserConsume(parser, TOKEN_COLON);
            parserConsume(parser, TOKEN_LPAREN);
            DynArray* argumentArray = arrayInit(sizeof(AST*));
            while (parser->type != TOKEN_RPAREN) { //Consume function argument types
                arrayAppend(argumentArray, parseDataType(parser, scope, parent));
                if (parser->type == TOKEN_COMMA) {
                    parserConsume(parser, TOKEN_COMMA);
                }
            }
            parserConsume(parser, TOKEN_RPAREN);
            AST* identifier = parseIdentifier(parser, scope, parent); //Construct function pointer AST
            ASTComp* argumentTypes = structAST(AST_COMP, 0, ASTComp, argumentArray);
            arrayAppend(nodes, structAST(AST_FUNC_VAR, 0, ASTFuncDef, dataType, identifier, argumentTypes, NULL));
            //TODO fix hack. this stops function pointers being supported by comma seperated definition without explicit data types.
            // this should be fixed at some point. Maybe function pointers need their own function instead of being inside parseDefinition?
            dataType = NULL;
        } else if (parser->type == TOKEN_ID) { //If we get here, we must be parsing a function or variable
            AST* identifier = parseIdentifier(parser, scope, parent);
            if (parser->type == TOKEN_LPAREN) { //Parsing Function Def
                parseFuncDefinition(nodes, parser, scope, dataType, identifier, parent);
                break; //Break out of loop, as no more work once function is constructed
            }
            parseVarDefinition(nodes, parser, scope, dataType, identifier, parent); //Finally, it must be a standard variable
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

void parseStructDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_STRUCT);
    ASTFlag flags = 0;
    if (parser->type == TOKEN_PACKED) { //Apply GCC data packing
        parserConsume(parser, TOKEN_PACKED);
        flags |= PACKED_DATA;
    }
    AST* id = parseIdentifier(parser, scope, parent);

    parserConsume(parser, TOKEN_LBRACE);
    int stackTop = scope->top;
    ASTStructDef* astStruct = structAST(AST_STRUCT, flags, ASTStructDef, id, NULL);
    ASTComp* members = parseAST(parser, scope, TOKEN_RBRACE, (AST*) astStruct);
    astStruct->members = members;
    arrayAppend(nodes, astStruct);

    //Push this new type to the definition table
    insert(scope->defs, viewToStr(&id->token->view), astStruct);

    scope->top = stackTop;
    parserConsume(parser, TOKEN_RBRACE);
}

void parseEnumDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_ENUM);
    ASTFlag flags = 0;
    if (parser->type == TOKEN_FLAG) { //Bitmask enum
        parserConsume(parser, TOKEN_FLAG);
        flags |= ENUM_FLAG;
    }
    if (parser->type == TOKEN_PACKED) { //Apply GCC data packing
        parserConsume(parser, TOKEN_PACKED);
        flags |= PACKED_DATA;
    }
    AST* identifier = parser->type == TOKEN_ID ? parseIdentifier(parser, scope, parent) : NULL;
    AST* dataType = NULL;
    if (parser->token->flags & DATA_TYPE) { //Enum has explicit value size
        if ((parser->token->flags & VAR_INT) == 0) { //Only allow integer value sizes
            ERROR("Enum constant type must be integer! %s (%s)", TOKEN_NAMES[parser->type], viewToStr(&parser->token->view))
        }
        dataType = parseDataType(parser, scope, parent);
    }
    parserConsume(parser, TOKEN_LBRACE);
    DynArray* constantArray = arrayInit(sizeof(AST*));
    size_t constantValue = 0; //Initial enum constant value
    while (parser->type == TOKEN_ID) {
        AST* constant = parseIdentifier(parser, scope, parent);
        if (parser->type == TOKEN_ASSIGNMENT) { //Assigning value to enum constant
            parserConsume(parser, TOKEN_ASSIGNMENT);
            AST* expression = parseExpression(parser, scope, parent);
            arrayAppend(nodes, structAST(AST_ASSIGN, 0, ASTAssign, constant, expression));
        } else { //No assigment, so default generated value
            if (flags & ENUM_FLAG) { //If it's a flag value, auto increment ^2
                ASTLiteral* integer = valueAST(AST_INTEGER, 0, value, 1 << constantValue);
                arrayAppend(constantArray, structAST(AST_ASSIGN, 0, ASTAssign, constant, (AST*) integer));
            } else { //Normal enum constant, no assignment or flag
                arrayAppend(constantArray, constant);
            }
        }
        if (parser->type == TOKEN_COMMA) parserConsume(parser, TOKEN_COMMA);
        if (parser->type == TOKEN_RBRACE) break;
        constantValue++;
    }
    parserConsume(parser, TOKEN_RBRACE);
    ASTComp* constants = structAST(AST_COMP, 0, ASTComp, constantArray); //TODO move this to parseAST? like struct members?
    arrayAppend(nodes, structAST(AST_ENUM, flags, ASTEnumDef, identifier, dataType, constants));
}

void parseUnionDefinition(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_UNION);
    AST* identifier = parser->type == TOKEN_ID ? identifier = parseIdentifier(parser, scope, parent) : NULL;
    parserConsume(parser, TOKEN_LBRACE);
    DynArray* memberArray = arrayInit(sizeof(AST*));
    //TODO
    parserConsume(parser, TOKEN_RBRACE);
    ASTComp* members = structAST(AST_COMP, 0, ASTComp, memberArray);
    arrayAppend(nodes, structAST(AST_UNION, 0, ASTUnionDef, identifier, members));
}

void parseIf(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_IF);
    parserConsume(parser, TOKEN_LPAREN);
    AST* expression = parseExpression(parser, scope, parent);
    parserConsume(parser, TOKEN_RPAREN);
    parserConsume(parser, TOKEN_LBRACE);
    int stackTop = scope->top;
    ASTComp* body = parseAST(parser, scope, TOKEN_RBRACE, parent);
    scope->top = stackTop;
    parserConsume(parser, TOKEN_RBRACE);
    arrayAppend(nodes, structAST(AST_IF, 0, ASTIf, expression, body));
}

void parseElse(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_ELSE);
    if (parser->type == TOKEN_LBRACE) {
        parserConsume(parser, TOKEN_LBRACE);
        int stackTop = scope->top;
        ASTComp* body = parseAST(parser, scope, TOKEN_RBRACE, parent);
        scope->top = stackTop;
        arrayAppend(nodes, structAST(AST_ELSE, 0, ASTElse, body));
        parserConsume(parser, TOKEN_RBRACE);
    } else {
        arrayAppend(nodes, structAST(AST_ELSE, 0, ASTElse, NULL));
    }
}

void parseWhile(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_WHILE);
    parserConsume(parser, TOKEN_LPAREN);
    AST* expression = parseExpression(parser, scope, parent);
    parserConsume(parser, TOKEN_RPAREN);
    parserConsume(parser, TOKEN_LBRACE);
    ASTComp* body = parseAST(parser, scope, TOKEN_RBRACE, parent);
    parserConsume(parser, TOKEN_RBRACE);
    arrayAppend(nodes, structAST(AST_WHILE, 0, ASTWhile, expression, body));
}

void parseFor(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    parserConsume(parser, TOKEN_FOR);
    parserConsume(parser, TOKEN_LPAREN);
    int stackTop = scope->top;
    DynArray* array = arrayInit(sizeof(AST*));
    parseDefinition(array, parser, scope, parent);
    //TODO hack
    AST* definition = (AST*) array->elements[0];
    //EOS consume not needed as parseVarDef will consume it
    //parserConsume(parser, TOKEN_EOS);
    AST* condition = parseExpression(parser, scope, parent);
    parserConsume(parser, TOKEN_EOS);
    ASTComp* astComp = parseAST(parser, scope, TOKEN_RPAREN, parent);
    AST* expression = astComp->array->elements[0];
    parserConsume(parser, TOKEN_RPAREN);
    parserConsume(parser, TOKEN_LBRACE);
    ASTComp* body = parseAST(parser, scope, TOKEN_RBRACE, parent);
    parserConsume(parser, TOKEN_RBRACE);
    scope->top = stackTop;
    arrayAppend(nodes, structAST(AST_FOR, 0, ASTFor, definition, condition, expression, body));
}

void parseBreak(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    Token* token = parser->token;
    parserConsume(parser, TOKEN_BREAK);
    parserConsume(parser, TOKEN_EOS);
    arrayAppend(nodes, basicAST(AST_BREAK, 0, token));
}

void parseIdentifierRef(DynArray* nodes, Parser* parser, Scope* scope, AST* parent) {
    AST* identifierOrType = parseIdentifierOrType(parser, scope, parent);
    if (parser->type == TOKEN_STAR) { //Pointer type
        parserConsume(parser, TOKEN_STAR);
        identifierOrType->flags |= POINTER_TYPE;
    }
    if (parser->type == TOKEN_ID) { //If the next token is another id, then it must be a new type being instantiated
        parseVarDefinition(nodes, parser, scope, identifierOrType, NULL, parent);
    } else if (parser->type == TOKEN_ASSIGNMENT) {
        parserConsume(parser, TOKEN_ASSIGNMENT);
        AST* right = parseExpression(parser, scope, parent);
        parserConsume(parser, TOKEN_EOS);
        arrayAppend(nodes, structAST(AST_ASSIGN, 0, ASTAssign, identifierOrType, right));
    } else if (parser->token->flags & TYPE_UNARY) { //Parsing Unary operators where the op second (++, --, etc)
        Token* token = parser->token;
        parserConsume(parser, parser->type);
        arrayAppend(nodes, structAST(AST_UNARY, UNARY_LEFT, ASTUnary, identifierOrType, token));
        if (parser->type == TOKEN_EOS) {
            //TODO bit of a hack, stops for loop requiring trailing EOS
            parserConsume(parser, TOKEN_EOS);
        }
    } else if (parser->type == TOKEN_LPAREN) { //This must be a void function call outside an expression
        AST* funcCall = parseFunctionCall(parser, scope, parent, identifierOrType, NULL, NULL);
        funcCall->flags |= NON_EXPR_FUNC;
        arrayAppend(nodes, funcCall);
        parserConsume(parser, TOKEN_EOS);
    }
}

ASTComp* parseAST(Parser* parser, Scope* scope, TokenType breakToken, AST* parent) {
    DynArray* nodes = arrayInit(sizeof(AST*));
    while (parser->type != breakToken) {
        if (parser->token->flags & DATA_TYPE) {
            parseDefinition(nodes, parser, scope, parent);
        } else if (parser->token->flags & TYPE_UNARY) {
            //Parsing Unary operators where the op comes first (!condition, -5, etc)
            AST* expression = parseExpression(parser, scope, parent);
            arrayAppend(nodes, /*structAST(AST_UNARY, 0, ASTUnary, expression, token)*/expression);
        } else if (parser->type == TOKEN_STRUCT) {
            parseStructDefinition(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_ENUM) {
            parseEnumDefinition(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_UNION) {
            parseUnionDefinition(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_C_STATEMENT) {
            parseCStatement(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_RETURN) {
            parseReturn(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_IMPORT) {
            parseImport(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_IF) {
            parseIf(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_ELSE) {
            parseElse(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_WHILE) {
            parseWhile(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_FOR) {
            parseFor(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_BREAK) {
            parseBreak(nodes, parser, scope, parent);
        } else if (parser->type == TOKEN_ID) {
            parseIdentifierRef(nodes, parser, scope, parent);
        }

        else {
            ERROR("Token Was Not Consumed Or Parsed! %s (%s)", TOKEN_NAMES[parser->type], viewToStr(&parser->token->view));
        }
    }
    return structAST(AST_COMP, 0, ASTComp, nodes);
}
#endif