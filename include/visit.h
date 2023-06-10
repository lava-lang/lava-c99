#ifndef LAVA_VISIT_H
#define LAVA_VISIT_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* node, OutputBuffer* buffer);

void visitNode(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, node->token->value);
}

void visitCompound(ASTCompound* node, OutputBuffer* buffer) {
    for (int i = 0; i < node->children->len; ++i) {
        visit((AST*) node->children->elements[i], buffer);
        bufferAppend(buffer, ";\n");
    }
}

void visitDataType(AST* node, OutputBuffer* buffer) {
    if (node->token->type == TOKEN_INT || node->token->type == TOKEN_I32) {
        bufferAppend(buffer, "int");
    } else if (node->token->type == TOKEN_I64) {
        bufferAppend(buffer, "long");
    } else if (node->token->type == TOKEN_FLOAT || node->token->type == TOKEN_F32) {
        bufferAppend(buffer, "float");
    } else if (node->token->type == TOKEN_F64) {
        bufferAppend(buffer, "double");
    }
}

void visitVarDefinition(ASTVarDef* varDef, OutputBuffer* buffer) {
    visit(varDef->dataType, buffer);
    bufferAppend(buffer, " ");
    visit(varDef->identifier, buffer);
    bufferAppend(buffer, " = ");
    visit(varDef->expression, buffer);
}

void visit(AST* node, OutputBuffer* buffer) {
    if (node == NULL) {
        return;
    }

    if (node->type == AST_COMPOUND) {
        return visitCompound((ASTCompound*) node, buffer);
    } else if (node->type == AST_DATA_TYPE) {
        return visitDataType(node, buffer);
    } else if (node->type == AST_IDENTIFIER) {
        return visitNode(node, buffer);
    } else if (node->type == AST_INTEGER_VALUE) {
        return visitNode(node, buffer);
    } else if (node->type == AST_FLOAT_VALUE) {
        return visitNode(node, buffer);
    }

    else if (node->type == AST_VAR_DEF) {
        visitVarDefinition((ASTVarDef*) node, buffer);
        return;
    } else {
        fprintf(stderr, "Unhandled AST: %s for %s\n", AST_NAMES[node->type], TOKEN_NAMES[node->token->type]);
        exit(1);
    }
}

char* generateCode(OutputBuffer* buffer) {
    return buffer->code;
}

#endif //LAVA_VISIT_H
