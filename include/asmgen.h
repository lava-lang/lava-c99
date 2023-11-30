#ifndef LAVA_ASMGEN_H
#define LAVA_ASMGEN_H

//TODO Maybe implement for basic Lava MVP

#include "structures.h"
#include "ast_old.h"

OutputBuffer* generateASM(AST* root) {
    OutputBuffer* outputBuffer = bufferInit();
    //visit(root, outputBuffer);
    return outputBuffer;
}
#endif //LAVA_ASMGEN_H
