#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* node, OutputBuffer* buffer);

void visitNode(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, node->token->value);
}

void visitCompound(ASTCompound* node, OutputBuffer* buffer) {
    for (size_t i = 0; i < node->children->len; ++i) {
        bufferAppendIndent(buffer);
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
        bufferAppend(buffer, "char");
    } else if (node->token->type == TOKEN_BOOLEAN) {
        bufferAppend(buffer, "bool");
        //TODO move to bootstrap code
        bufferAddImport(buffer, "<stdbool.h>");
    } else {
        PANIC("Unhandled DataType: %s for %s", AST_NAMES[node->astType], TOKEN_NAMES[node->token->type]);
    }
}

void visitVarDefinition(ASTVarDef* varDef, OutputBuffer* buffer) {
    visit(varDef->dataType, buffer);
    if (((TokenVar*) varDef->dataType->token)->isPointer) {
        bufferAppend(buffer, "*");
    }
    bufferAppend(buffer, " ");
    visit(varDef->identifier, buffer);
    bufferAppend(buffer, " = ");
    if (varDef->dataType->token->type == TOKEN_STRING) { //Handle string quotes
        bufferAppend(buffer, "\"");
        visit(varDef->expression, buffer);
        bufferAppend(buffer, "\"");
    } else {
        visit(varDef->expression, buffer);
    }
    bufferAppend(buffer, ";");
    bufferAppend(buffer, "\n");
}

void visitFuncDefinition(ASTFuncDef* funcDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    visit(funcDef->returnType, buffer);
    bufferAppend(buffer, " ");
    visit(funcDef->identifier, buffer);
    bufferAppend(buffer, "(");
    //TODO args..
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound((ASTCompound*) funcDef->compound, buffer);
    bufferAppend(buffer, "\n}");
    bufferUnindent(buffer);
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
    bufferAppend(buffer, "\n");
}

void visitReturn(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visitNode(((ASTReturn*) node)->expression, buffer);
    bufferAppend(buffer, ";");
}

void visitImport(AST* node, OutputBuffer* buffer) {
    if (strstr(node->token->value, ".h") || node->token->value[0] == '<' || node->token->value[0] == '"') { //C Import
        bufferAddImport(buffer, node->token->value);
    } else { //Lava import
        char* value = charToStr('"');
        value = concatStr(value, node->token->value);
        value = concatStr(value, ".h\"");
        bufferAddImport(buffer, value);
    }
}

void visit(AST* node, OutputBuffer* buffer) {
    if (node == NULL) {
        return;
    }

    if (node->astType == AST_COMPOUND) {
        return visitCompound((ASTCompound*) node, buffer);
    } else if (node->astType == AST_DATA_TYPE) {
        return visitDataType(node, buffer);
    } else if (node->astType == AST_IDENTIFIER) {
        return visitNode(node, buffer);
    } else if (node->astType == AST_VAR_VALUE) {
        return visitNode(node, buffer);
    }

    else if (node->astType == AST_VAR_DEF) {
        return visitVarDefinition((ASTVarDef*) node, buffer);
    } else if (node->astType == AST_FUNC_DEF) {
        return visitFuncDefinition((ASTFuncDef*) node, buffer);
    }

    else if (node->astType == AST_C_STATEMENT) {
        return visitCStatement(node, buffer);
    } else if (node->astType == AST_RETURN) {
        return visitReturn(node, buffer);
    } else if (node->astType == AST_IMPORT) {
        return visitImport(node, buffer);
    }

    else {
        PANIC("Unhandled AST: %s for %s", AST_NAMES[node->astType], TOKEN_NAMES[node->token->type]);
    }
}

OutputBuffer* generateC(AST* root) {
    OutputBuffer* outputBuffer = bufferInit();
    visit(root, outputBuffer);
    return outputBuffer;
}
#endif //LAVA_CGEN_H