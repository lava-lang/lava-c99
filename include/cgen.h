#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* parent, AST* node, OutputBuffer* buffer);

void visitNode(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token->view);
}

void visitCompound(AST* parent, AST* node, OutputBuffer* buffer, char* delimiter, bool skipLastDelim) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit(parent, (AST*) node->array->elements[i], buffer);
        //TODO remove AST_IMPORT exclusion somehow
        if ((i < node->array->len - 1 || !skipLastDelim) && ((AST*) node->array->elements[i])->type != AST_IMPORT) {
            bufferAppend(buffer, delimiter);
        }
    }
}

void visitDataType(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token->view);
    if (node->token->flags & VAR_POINTER) {
        bufferAppend(buffer, "*");
    }
}

void visitVarDefinition(AST* parent, AST* node, OutputBuffer* buffer) {
    visit(node, node->varDef->dataType, buffer);
    bufferAppend(buffer, " ");
    visit(node, node->varDef->identifier, buffer);
    if (node->varDef->dataType->token->flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (node->varDef->expression) {
        bufferAppend(buffer, " = ");
        if (node->varDef->dataType->token->type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(node, node->varDef->expression, buffer);
            bufferAppend(buffer, "\"");
        } else if (node->varDef->dataType->token->type == TOKEN_CHAR) { //Handle char quotes
            bufferAppend(buffer, "\'");
            visit(node, node->varDef->expression, buffer);
            bufferAppend(buffer, "\'");
        } else {
            visit(node, node->varDef->expression, buffer);
        }
    }
    if (!(node->flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visitStructDefinition(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nstruct ");
//    if (node->flags & PACKED_DATA) {
//        bufferAppend(buffer, "__attribute__((__packed__)) ");
//    }
    visit(node, node->structDef->identifier, buffer);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
    visitCompound(node, node->structDef->members, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    bufferAppendPrefix(buffer, "\ntypedef struct "); //Hoist struct definition
    bufferAppendPrefixView(buffer, &node->structDef->identifier->token->view);
    bufferAppendPrefix(buffer, " ");
    bufferAppendPrefixView(buffer, &node->structDef->identifier->token->view);
    bufferAppendPrefix(buffer, ";");

    FREE(node->structDef->members->array); //Cleanup array allocations
}

void visitEnumDefinition(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nenum ");
//    if (node->flags & PACKED_DATA) {
//        bufferAppend(buffer, "__attribute__((__packed__)) ");
//    }
//    if (node->enumDef->identifier) {
        visit(node, node->enumDef->identifier, buffer);
        bufferAppend(buffer, " ");
//    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node, node->enumDef->constants, buffer, ",\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    if (node->enumDef->identifier) { //Hoist enum definition
        bufferAppendPrefix(buffer, "\ntypedef enum ");
        bufferAppendPrefixView(buffer, &node->enumDef->identifier->token->view);
        bufferAppendPrefix(buffer, " ");
        bufferAppendPrefixView(buffer, &node->enumDef->identifier->token->view);
        bufferAppendPrefix(buffer, ";");
    }

    FREE(node->enumDef->constants->array); //Cleanup array allocations
}

void visitFuncDefinition(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    size_t bufStartPos = strlen(buffer->code);
    visit(node, node->funcDef->returnType, buffer);
    bufferAppend(buffer, " ");
    visit(node, node->funcDef->identifier, buffer);
    bufferAppend(buffer, "(");
    visitCompound(node, node->funcDef->arguments, buffer, ", ", true);
    bufferAppend(buffer, ")");
    size_t bufEndPos = strlen(buffer->code);
    bufferAppend(buffer, " {\n");

    bufferIndent(buffer);
    visitCompound(node, node->funcDef->statements, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");

    size_t bufSize = bufEndPos - bufStartPos; //Build function definition to hoist
    char* hoist = calloc(bufSize + 2, sizeof(char)); //TODO replace with view
    strncpy(hoist, buffer->code+bufStartPos, bufSize);
    hoist[bufSize] = ';';
    hoist[bufSize + 1] = '\0';
    bufferAppendPrefix(buffer, "\n");
    bufferAppendPrefix(buffer, hoist);

    FREE(node->funcDef->arguments->array); //Cleanup array allocations
    FREE(node->funcDef->statements->array);
}

void visitUnionDefinition(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nunion ");
    if (node->flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    if (node->unionDef->identifier) {
        visit(node, node->unionDef->identifier, buffer);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node, node->unionDef->members, buffer, ";\n", false);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    if (node->unionDef->identifier) { //Hoist union definition
        bufferAppendPrefix(buffer, "\ntypedef union ");
        bufferAppendPrefixView(buffer, &node->unionDef->identifier->token->view);
        bufferAppendPrefix(buffer, " ");
        bufferAppendPrefixView(buffer, &node->unionDef->identifier->token->view);
        bufferAppendPrefix(buffer, ";");
    }

    FREE(node->unionDef->members->array); //Cleanup array allocations
}

void visitCStatement(AST* parent, AST* node, OutputBuffer* buffer) {
    visitNode(node, node, buffer);
}

void visitReturn(AST* parent, AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visitNode(node, node->expression, buffer);
    bufferAppend(buffer, ";");
}

void visitImport(AST* parent, AST* node, OutputBuffer* buffer) {
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

void visitBinop(AST* parent, AST* node, OutputBuffer* buffer) {
    visit(node, node->binop->left, buffer);
    bufferAppend(buffer, " ");
    bufferAppendView(buffer, &node->binop->operator->view);
    bufferAppend(buffer, " ");
    visit(node, node->binop->right, buffer);
}

void visitAssign(AST* parent, AST* node, OutputBuffer* buffer) {
    visit(node, node->assign->left, buffer);
    bufferAppend(buffer, " = ");
    visit(node, node->assign->right, buffer);
}

void visitInteger(AST* parent, AST* node, OutputBuffer* buffer) {
    char str[256] = "";
    snprintf(str, sizeof(str), "%zu", node->value);
    bufferAppend(buffer, str);
}

void visitFuncVar(AST* parent, AST* node, OutputBuffer* buffer) {
    visit(node, node->funcDef->returnType, buffer);
    bufferAppend(buffer, " (*");
    visit(node, node->funcDef->identifier, buffer);
    bufferAppend(buffer, ")(");
    //Function argument types
    visitCompound(node, node->funcDef->arguments, buffer, ", ", true);
    bufferAppend(buffer, ")");

    FREE(node->funcDef->arguments->array); //Cleanup array allocations

    if (!(node->flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visit(AST* parent, AST* node, OutputBuffer* buffer) {
    if (node == NULL) return;
    switch (node->type) {
        case AST_COMP: visitCompound(parent, node, buffer, "\n", false); break;
        case AST_TYPE: visitDataType(parent, node, buffer); break;
        case AST_ID: case AST_VALUE: visitNode(parent, node, buffer); break;
        case AST_VAR: visitVarDefinition(parent, node, buffer); break;
        case AST_STRUCT: visitStructDefinition(parent, node, buffer); break;
        case AST_ENUM: visitEnumDefinition(parent, node, buffer); break;
        case AST_FUNC: visitFuncDefinition(parent, node, buffer); break;
//        case AST_UNION: visitUnionDefinition(parent, node, buffer); break;
        case AST_C: visitCStatement(parent, node, buffer); break;
        case AST_RETURN: visitReturn(parent, node, buffer); break;
        case AST_IMPORT: visitImport(parent, node, buffer); break;
        case AST_BINOP: visitBinop(parent, node, buffer); break;
        case AST_ASSIGN: visitAssign(parent, node, buffer); break;
        case AST_INTEGER: visitInteger(parent, node, buffer); break;
//        case AST_FUNC_VAR: visitFuncVar(parent, node, buffer); break;
        default: PANIC("Unhandled AST: %s %s", AST_NAMES[node->type], node->token->type != TOKEN_NONE_ ? TOKEN_NAMES[node->token->type] : "");
    }
}

OutputBuffer* generateC(AST* root) {
    OutputBuffer* buffer = bufferInit();
    bufferAppend(buffer, "\n\n"); //Initial \n\n to space definitions
    visit(NULL, root, buffer);
    return buffer;
}
#endif //LAVA_CGEN_H