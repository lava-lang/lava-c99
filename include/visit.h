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
    }
}

void visitDataType(AST* node, OutputBuffer* buffer) {
    if (node->token->type == TOKEN_VOID) {
        bufferAppend(buffer, "void");
    } else if (node->token->type == TOKEN_INT || node->token->type == TOKEN_I32) {
        bufferAppend(buffer, "int");
    } else if (node->token->type == TOKEN_I64) {
        bufferAppend(buffer, "long");
    } else if (node->token->type == TOKEN_FLOAT || node->token->type == TOKEN_F32) {
        bufferAppend(buffer, "float");
    } else if (node->token->type == TOKEN_F64) {
        bufferAppend(buffer, "double");
    } else if (node->token->type == TOKEN_STRING) {
        bufferAppend(buffer, "char*");
    } else {
        fprintf(stderr, "Unhandled DataType: %s for %s\n", AST_NAMES[node->type], TOKEN_NAMES[node->token->type]);
        exit(1);
    }
}

void visitVarDefinition(ASTVarDef* varDef, OutputBuffer* buffer) {
    visit(varDef->dataType, buffer);
    bufferAppend(buffer, " ");
    visit(varDef->identifier, buffer);
    bufferAppend(buffer, " = ");
    visit(varDef->expression, buffer);
}

void visitFuncDefinition(ASTFuncDef* funcDef, OutputBuffer* buffer) {
    visit(funcDef->returnType, buffer);
    bufferAppend(buffer, " ");
    visit(funcDef->identifier, buffer);
    bufferAppend(buffer, "(");
    //TODO args..
    bufferAppend(buffer, ") {\n");
    //TODO compound...
    bufferAppend(buffer, "}");
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
    } else if (node->type == AST_VAR_VALUE) {
        return visitNode(node, buffer);
    }

    else if (node->type == AST_VAR_DEF) {
        visitVarDefinition((ASTVarDef*) node, buffer);
        bufferAppend(buffer, ";\n");
        return;
    } else if (node->type == AST_FUNC_DEF) {
        return visitFuncDefinition((ASTFuncDef*) node, buffer);
    }

    else {
        fprintf(stderr, "Unhandled AST: %s for %s\n", AST_NAMES[node->type], TOKEN_NAMES[node->token->type]);
        exit(1);
    }
}

char* generateCode(OutputBuffer* buffer) {
    return buffer->code;
}

#endif //LAVA_VISIT_H
