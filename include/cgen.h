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

void visitCompound(AST* node, OutputBuffer* buffer, char* delimiter) {
    for (size_t i = 0; i < node->comp.array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->comp.array->elements[i], buffer);
        //TODO remove AST_IMPORT exclusion somehow
        if (i < node->comp.array->len - 1 && ((AST*) node->comp.array->elements[i])->type != AST_IMPORT) {
            bufferAppend(buffer, delimiter);
        }
    }
}

void visitVarDefinition(AST* node, OutputBuffer* buffer, bool arg) {
    visit(node->varDef.dataType, buffer);
    if (node->varDef.dataType->token->flags & VAR_POINTER) {
        bufferAppend(buffer, "*"); //TODO move to visitDataType?
    }
    bufferAppend(buffer, " ");
    visit(node->varDef.identifier, buffer);
    if (node->varDef.dataType->token->flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (node->varDef.expression) {
        bufferAppend(buffer, " = ");
        if (node->varDef.dataType->token->type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(node->varDef.expression, buffer);
            bufferAppend(buffer, "\"");
        } else if (node->varDef.dataType->token->type == TOKEN_CHAR) { //Handle char quotes
            bufferAppend(buffer, "\'");
            visit(node->varDef.expression, buffer);
            bufferAppend(buffer, "\'");
        } else {
            visit(node->varDef.expression, buffer);
        }
    }
    if (!arg) { //If this is not a function argument
        bufferAppend(buffer, ";");
    }
}

void visitStructDefinition(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nstruct ");
    visit(node->structDef.identifier, buffer);
    bufferAppend(buffer, "_t {\n");
    bufferIndent(buffer);
    visitCompound(node->structDef.members, buffer, "\n");
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    //Hoist struct definition
    bufferAppendPrefix(buffer, "\ntypedef struct ");
    bufferAppendPrefixView(buffer, &node->structDef.identifier->token->view);
    bufferAppendPrefix(buffer, "_t ");
    bufferAppendPrefixView(buffer, &node->structDef.identifier->token->view);
    bufferAppendPrefix(buffer, ";");

    //Cleanup array allocations
    FREE(node->structDef.members->comp.array);
}

void visitEnumDefinition(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nenum ");
    visit(node->enumDef.identifier, buffer);
    bufferAppend(buffer, "_t {\n");
    bufferIndent(buffer);
    visitCompound(node->enumDef.constants, buffer, ",\n");
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    //Hoist struct definition
    bufferAppendPrefix(buffer, "\ntypedef enum ");
    bufferAppendPrefixView(buffer, &node->enumDef.identifier->token->view);
    bufferAppendPrefix(buffer, "_t ");
    bufferAppendPrefixView(buffer, &node->enumDef.identifier->token->view);
    bufferAppendPrefix(buffer, ";");

    //Cleanup array allocations
    FREE(node->enumDef.constants->comp.array);
}

void visitFuncDefinition(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    size_t bufStartPos = strlen(buffer->code);
    visit(node->funcDef.returnType, buffer);
    bufferAppend(buffer, " ");
    visit(node->funcDef.identifier, buffer);
    bufferAppend(buffer, "(");
    //Arguments
    for (int i = 0; i < node->funcDef.arguments->comp.array->len; ++i) {
        bufferAppendIndent(buffer);
        visitVarDefinition(node->funcDef.arguments->comp.array->elements[i], buffer, true);
        if (i < node->funcDef.arguments->comp.array->len - 1) {
            bufferAppend(buffer, ", ");
        }
    }
    //TODO make work..
    //visitCompound(node->funcDef.arguments, buffer, ", ");
    bufferAppend(buffer, ")");
    size_t bufEndPos = strlen(buffer->code);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
    visitCompound(node->funcDef.statements, buffer, "\n");
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

    FREE(node->funcDef.arguments->comp.array); //Cleanup array allocations
    FREE(node->funcDef.statements->comp.array);
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
}

void visitReturn(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visitNode(node->returnDef.expression, buffer);
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

    if (node->type == AST_COMP) {
        visitCompound(node, buffer, "\n");
    } else if (node->type == AST_TYPE || node->type == AST_ID || node->type == AST_VALUE) {
        visitNode(node, buffer);
    }

    else if (node->type == AST_VAR) {
        visitVarDefinition(node, buffer, false);
    } else if (node->type == AST_STRUCT) {
        visitStructDefinition(node, buffer);
    } else if (node->type == AST_ENUM) {
        visitEnumDefinition(node, buffer);
    } else if (node->type == AST_FUNC) {
        visitFuncDefinition(node, buffer);
    }

    else if (node->type == AST_C) {
        visitCStatement(node, buffer);
    } else if (node->type == AST_RETURN) {
        visitReturn(node, buffer);
    } else if (node->type == AST_IMPORT) {
        visitImport(node, buffer);
    } else if (node->type == AST_BINOP) {
        printf("%s\n", viewToStr(&node->token->view));
        visit(node->binop.left, buffer);
        bufferAppend(buffer, " ");
        bufferAppendView(buffer, &node->token->view);
        bufferAppend(buffer, " ");
        visit(node->binop.right, buffer);
    } else if (node->type == AST_ASSIGN) {
        visit(node->assign.left, buffer);
        bufferAppend(buffer, " = ");
        visit(node->assign.right, buffer);
    } else if (node->type == AST_INTEGER) {
        char str[256] = "";
        snprintf(str, sizeof(str), "%zu", node->integer.value);
        bufferAppend(buffer, str);
    }

    else {
        PANIC("Unhandled AST: %s %s", AST_NAMES[node->type], node->token->type != TOKEN_NONE_ ? TOKEN_NAMES[node->token->type] : "");
    }
}

OutputBuffer* generateC(AST* root) {
    OutputBuffer* buffer = bufferInit();
    bufferAppend(buffer, "\n\n"); //Initial \n\n to space definitions
    visit(root, buffer);
    return buffer;
}
#endif //LAVA_CGEN_H