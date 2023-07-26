#include <stdio.h>
#include <time.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/structures.h"
#include "include/file.h"
#include "include/ast.h"
#include "include/cgen.h"
#include "include/region.h"

int main(int argc, char *argv[]) {
    //Begin profiling
    clock_t startAll = clock();

    //Init virtual memory region
    clock_t startArena = clock();
    GLOBAL_REGION_CAPACITY = 10000;
    initGlobalRegion(CALLOC(1, GLOBAL_REGION_CAPACITY));
    BASIC("Arena: %f", (double)(clock() - startArena) / CLOCKS_PER_SEC)

    //Exit if arguments is less than 2
    ASSERT(argc < 2, "Two arguments required, %d were passed.", argc)

    clock_t startLoad = clock();
    char* inputCode = read_file(argv[1]);
    BASIC("Load: %f", (double)(clock() - startLoad) / CLOCKS_PER_SEC)
    DEBUG("Lava Input Code:\n%s\n", inputCode)

    clock_t startParse = clock();
    Scope* globalScope = scopeInit(NULL);
    Lexer* lexer = lexerInit(argv[1], inputCode);
    Parser* parser = parserInit(lexer);
    AST* root = parseAST(parser, globalScope, TOKEN_EOF);
    BASIC("Parsing: %f", (double)(clock() - startParse) / CLOCKS_PER_SEC)

    #if DEBUG_MODE == 1
        for (int i = 0; i < root->comp.array->len; ++i) {
            AST* node = (AST*) root->comp.array->elements[i];
            printAST(node, 0);
        }
    #endif

    clock_t startCodegen = clock();
    OutputBuffer* outputBuffer = generateC(root);
    char* generatedCode = bufferBuild(outputBuffer);
    DEBUG("C Code Generation:\n%s\n", generatedCode)
    BASIC("Codegen: %f", (double)(clock() - startCodegen) / CLOCKS_PER_SEC)

    //Write generated C file to disk
    clock_t startWrite = clock();
    write_file("../output.c", generatedCode);
    BASIC("Write: %f", (double)(clock() - startWrite) / CLOCKS_PER_SEC)
    BASIC("Full: %f\n", (double)(clock() - startAll) / CLOCKS_PER_SEC)

    //Free memory allocations
    FREE(inputCode);
    //scopeFree(globalScope);
    FREE(root->comp.array);
    bufferFree(outputBuffer);

    //Make sure there are no leaks
    freeGlobalRegion();
    BASIC("Tokens Consumed: %d", TOKENS_CONSUMED)
    BASIC("AST Nodes Constructed: %d", AST_NODES_CONSTRUCTED)
    BASIC("ALLOCATIONS: %zu/%zu", ALLOC_COUNT, FREE_COUNT);

    return 0;
}