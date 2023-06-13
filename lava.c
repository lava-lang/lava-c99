#include <stdio.h>
#include <stdlib.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/structures.h"
#include "include/file.h"
#include "include/debug.h"
#include "include/ast.h"
#include "include/visit.h"

int main(int argc, char *argv[]) {
    //Exit if arguments is less than 2
    if (argc < 2) {
        fprintf(stderr, "Lava compiler requires two arguments!");
        exit(1);
    }

    printf("Lava Input Code:\n");
    char* inputCode = read_file(argv[1]);
    printf("%s\n\n", inputCode);

    Scope* globalScope = scopeInit((void*) 0);
    Lexer* lexer = lexerInit(inputCode);
    Parser* parser = parserInit(lexer);
    ASTCompound* astNodes = parseAST(parser, globalScope);
    printf("Tokens Consumed: %d\n\n", TOKENS_CONSUMED);

    for (int i = 0; i < astNodes->children->len; ++i) {
        AST* node = (AST*) astNodes->children->elements[i];
        printf("Node: %s\n", AST_NAMES[node->type]);

        if (node->type == AST_VAR_DEF) {
            printVarDef((ASTVarDef*) node);
        } else if (node->type == AST_FUNC_DEF) {
            printFuncDef((ASTFuncDef*) node);
        }
    }
    printf("AST Nodes Constructed: %d\n\n", AST_NODES_CONSTRUCTED);

    OutputBuffer* outputBuffer = bufferInit();
    visit((AST*) astNodes, outputBuffer);
    char* generatedCode = generateCode(outputBuffer);
    printf("C Code Generation:\n");
    printf("%s\n", generatedCode);

    //Free memory allocations
    bufferFree(outputBuffer);
    parserFree(parser);
    scopeFree(globalScope);

    return 0;
}