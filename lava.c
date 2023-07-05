#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/structures.h"
#include "include/file.h"
#include "include/debug.h"
#include "include/ast.h"
#include "include/visit.h"

int main(int argc, char *argv[]) {
    //Begin profiling
    clock_t startAll = clock();

    //Exit if arguments is less than 2
    ASSERT(argc < 2, "Two arguments required, %d were passed.", argc)

    printf("Lava Input Code:\n");
    char* inputCode = read_file(argv[1]);
    printf("%s\n\n", inputCode);

    clock_t startParse = clock();
    Scope* globalScope = scopeInit((void*) 0);
    Lexer* lexer = lexerInit(argv[1], inputCode);
    Parser* parser = parserInit(lexer);
    ASTCompound* ast = (ASTCompound*) parseAST(parser, globalScope, TOKEN_EOF);
    #ifdef DEBUG
        printf("Tokens Consumed: %d\n\n", TOKENS_CONSUMED);
    #endif

    for (int i = 0; i < ast->children->len; ++i) {
        AST* node = (AST*) ast->children->elements[i];
        #ifdef DEBUG
            printf("Node: %s\n", AST_NAMES[node->astType]);
        #endif

        if (node->astType == AST_VAR_DEF) {
            printVarDef((ASTVarDef*) node);
        } else if (node->astType == AST_FUNC_DEF) {
            printFuncDef((ASTFuncDef*) node);
        }
    }
    clock_t endParse = clock();
    printf("AST Nodes Constructed: %d\n\n", AST_NODES_CONSTRUCTED);

    clock_t startCodegen = clock();
    OutputBuffer* outputBuffer = bufferInit();
    visit((AST*) ast, outputBuffer);
    char* generatedCode = bufferBuild(outputBuffer);
    printf("C Code Generation:\n");
    printf("%s\n", generatedCode);
    clock_t endCodegen = clock();

    //Write generated C file to disk
    write_file("../output.c", generatedCode);

    clock_t endAll = clock();
    printf("\nParsing: %f\n", (double)(endParse - startParse) / CLOCKS_PER_SEC);
    printf("Codegen: %f\n", (double)(endCodegen - startCodegen) / CLOCKS_PER_SEC);
    printf("Full: %f\n", (double)(endAll - startAll) / CLOCKS_PER_SEC);

    //Free memory allocations
    bufferFree(outputBuffer);
    parserFree(parser);
    scopeFree(globalScope);

    return 0;
}