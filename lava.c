#include <stdio.h>
#include <time.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/structures.h"
#include "include/file.h"
#include "include/ast.h"
#include "include/cgen.h"

int main(int argc, char *argv[]) {
    //Begin profiling
    clock_t startAll = clock();

    //Exit if arguments is less than 2
    ASSERT(argc < 2, "Two arguments required, %d were passed.", argc)

    char* inputCode = read_file(argv[1]);
    DEBUG("Lava Input Code:\n%s\n", inputCode)

    clock_t startParse = clock();
    Scope* globalScope = scopeInit(NULL);
    Lexer* lexer = lexerInit(argv[1], inputCode);
    Parser* parser = parserInit(lexer);
    AST* root = parseAST(parser, globalScope, TOKEN_EOF);
    DEBUG("Tokens Consumed: %d\n", TOKENS_CONSUMED)

    #if DEBUG_MODE == 1
        ASTCompound* compound = (ASTCompound*) root;
        for (int i = 0; i < compound->array->len; ++i) {
            AST* node = (AST*) compound->array->elements[i];
            printAST(node, 0);
        }
    #endif

    clock_t endParse = clock();
    DEBUG("AST Nodes Constructed: %d\n", AST_NODES_CONSTRUCTED)

    clock_t startCodegen = clock();
    OutputBuffer* outputBuffer = generateC(root);
//    char* generatedCode = bufferBuild(outputBuffer);
//    DEBUG("C Code Generation:\n%s\n", generatedCode)
    clock_t endCodegen = clock();

    //Write generated C file to disk
//    write_file("../output.c", generatedCode);

    clock_t endAll = clock();
    BASIC("Parsing: %f", (double)(endParse - startParse) / CLOCKS_PER_SEC)
    BASIC("Codegen: %f", (double)(endCodegen - startCodegen) / CLOCKS_PER_SEC)
    BASIC("Full: %f\n", (double)(endAll - startAll) / CLOCKS_PER_SEC)

    //Free memory allocations

    scopeFree(globalScope);
    lexerFree(lexer);
    parserFree(parser);
//    astFree(root);
    bufferFree(outputBuffer);

    //Make sure there are no leaks
    printf("AST NODES CONSTRUCTED: %d; KILLED: %d\n", AST_NODES_CONSTRUCTED, FREE_NODE_COUNT);
    checkAllocations();

    return 0;
}