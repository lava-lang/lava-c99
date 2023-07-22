#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* node, OutputBuffer* buffer);

void visitNode(AST* node, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token->view);
}

void visitCompound(ASTCompound* node, OutputBuffer* buffer, char* delimiter) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->array->elements[i], buffer);
        //TODO remove AST_IMPORT exclusion somehow
        if (i < node->array->len - 1 && ((AST*) node->array->elements[i])->astType != AST_IMPORT) {
            bufferAppend(buffer, delimiter);
        }
    }
}

void visitVarDefinition(ASTVarDef* varDef, OutputBuffer* buffer, bool arg) {
    visit(varDef->dataType, buffer);
    if (varDef->dataType->token->flags & VAR_POINTER) {
        bufferAppend(buffer, "*"); //TODO move to visitDataType?
    }
    bufferAppend(buffer, " ");
    visit(varDef->identifier, buffer);
    if (varDef->dataType->token->flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (varDef->expression) {
        bufferAppend(buffer, " = ");
        if (varDef->dataType->token->type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(varDef->expression, buffer);
            bufferAppend(buffer, "\"");
        } else if (varDef->dataType->token->type == TOKEN_CHAR) { //Handle char quotes
            bufferAppend(buffer, "\'");
            visit(varDef->expression, buffer);
            bufferAppend(buffer, "\'");
        } else {
            visit(varDef->expression, buffer);
        }
    }
    if (!arg) { //If this is not a function argument
        bufferAppend(buffer, ";");
    }
}

void visitStructDefinition(ASTStructDef* structDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nstruct ");
    visit(structDef->identifier, buffer);
    bufferAppend(buffer, "_t {\n");
    bufferIndent(buffer);
        visitCompound(structDef->members, buffer, "\n");
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    //Hoist struct definition
    bufferAppendPrefix(buffer, "\ntypedef struct ");
    bufferAppendPrefixView(buffer, &structDef->identifier->token->view);
    bufferAppendPrefix(buffer, "_t ");
    bufferAppendPrefixView(buffer, &structDef->identifier->token->view);
    bufferAppendPrefix(buffer, ";");

    //Cleanup array allocations
    FREE(structDef->members->array);
}

void visitFuncDefinition(ASTFuncDef* funcDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    size_t bufStartPos = strlen(buffer->code);
    visit(funcDef->returnType, buffer);
    bufferAppend(buffer, " ");
    visit(funcDef->identifier, buffer);
    bufferAppend(buffer, "(");
    //Arguments
    for (int i = 0; i < funcDef->arguments->array->len; ++i) {
        bufferAppendIndent(buffer);
        visitVarDefinition((ASTVarDef*) funcDef->arguments->array->elements[i], buffer, true);
        if (i < funcDef->arguments->array->len - 1) {
            bufferAppend(buffer, ", ");
        }
    }
    //TODO make work..
    //visitCompound(funcDef->arguments, buffer, ", ");
    bufferAppend(buffer, ")");
    size_t bufEndPos = strlen(buffer->code);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
        visitCompound(funcDef->statements, buffer, "\n");
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");

    //Build function definition to hoist
    size_t bufSize = bufEndPos - bufStartPos;
    char* hoist = calloc(bufSize + 2, sizeof(char)); //TODO replace with view
    strncpy(hoist, buffer->code+bufStartPos, bufSize);
    hoist[bufSize] = ';';
    hoist[bufSize + 1] = '\0';
    bufferAppendPrefix(buffer, "\n");
    bufferAppendPrefix(buffer, hoist);

    FREE(funcDef->arguments->array); //Cleanup array allocations
    FREE(funcDef->statements->array);
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
}

void visitReturn(ASTReturn* returnDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visitNode(returnDef->expression, buffer);
    bufferAppend(buffer, ";");
}

void visitImport(AST* node, OutputBuffer* buffer) {
    if (node->token->view.start[0] == '<' || node->token->view.start[0] == '"') { //C Import
        bufferAppendPrefix(buffer, "#include ");
        bufferAppendPrefixView(buffer, &node->token->view);
        bufferAppendPrefix(buffer, "\n");
    } else { //Lava import
        bufferAppendPrefix(buffer, "#include \"");
        bufferAppendPrefixView(buffer, &node->token->view);
        bufferAppendPrefix(buffer, ".h\"\n");
    }
}

void visit(AST* node, OutputBuffer* buffer) {
    if (node == NULL) {
        return;
    }

    if (node->astType == AST_COMPOUND) {
        visitCompound((ASTCompound*) node, buffer, "\n");
    } else if (node->astType == AST_DATA_TYPE) {
        visitNode(node, buffer);
    } else if (node->astType == AST_IDENTIFIER || node->astType == AST_VAR_VALUE) {
        visitNode(node, buffer);
    }

    else if (node->astType == AST_VAR_DEF) {
        visitVarDefinition((ASTVarDef*) node, buffer, false);
    } else if (node->astType == AST_STRUCT_DEF) {
        visitStructDefinition((ASTStructDef *) node, buffer);
    } else if (node->astType == AST_FUNC_DEF) {
        visitFuncDefinition((ASTFuncDef*) node, buffer);
    }

    else if (node->astType == AST_C_STATEMENT) {
        visitCStatement(node, buffer);
    } else if (node->astType == AST_RETURN) {
        visitReturn((ASTReturn*) node, buffer);
    } else if (node->astType == AST_IMPORT) {
        visitImport(node, buffer);
    }

    else {
        PANIC("Unhandled AST: %s %s", AST_NAMES[node->astType], node->token->type != TOKEN_NONE ? TOKEN_NAMES[node->token->type] : "");
    }
}

OutputBuffer* generateC(AST* root) {
    OutputBuffer* buffer = bufferInit();
    bufferAppend(buffer, "\n\n"); //Initial \n\n to space definitions
    visit(root, buffer);
    return buffer;
}
#endif //LAVA_CGEN_H