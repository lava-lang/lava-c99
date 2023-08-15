#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* node, OutputBuffer* buffer);

void visitNode(AST* node, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token.view);
}

void visitCompound(ASTComp* node, OutputBuffer* buffer, char* delimiter, bool skipLastDelim) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->array->elements[i], buffer);
        //TODO remove AST_IMPORT exclusion somehow
        if ((i < node->array->len - 1 || !skipLastDelim) && ((AST*) node->array->elements[i])->type != AST_IMPORT) {
            bufferAppend(buffer, delimiter);
        }
    }
}

void visitDataType(AST* node, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token.view);
    if (node->token.flags & VAR_POINTER) {
        bufferAppend(buffer, "*");
    }
}

void visitVarDefinition(ASTVarDef* node, OutputBuffer* buffer) {
    visit(node->dataType, buffer);
    bufferAppend(buffer, " ");
    visit(node->identifier, buffer);
    if (node->dataType->token.flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (node->expression) {
        bufferAppend(buffer, " = ");
        if (node->dataType->token.type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(node->expression, buffer);
            bufferAppend(buffer, "\"");
        } else if (node->dataType->token.type == TOKEN_CHAR) { //Handle char quotes
            bufferAppend(buffer, "\'");
            visit(node->expression, buffer);
            bufferAppend(buffer, "\'");
        } else {
            visit(node->expression, buffer);
        }
    }
    if (!(node->base.flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visitStructDefinition(ASTStructDef* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nstruct ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    visit(node->identifier, buffer);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
    visitCompound(node->members, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    bufferAppendPrefix(buffer, "\ntypedef struct "); //Hoist struct definition
    bufferAppendPrefixView(buffer, &node->identifier->token.view);
    bufferAppendPrefix(buffer, " ");
    bufferAppendPrefixView(buffer, &node->identifier->token.view);
    bufferAppendPrefix(buffer, ";");

    FREE(node->members->array); //Cleanup array allocations
}

void visitEnumDefinition(ASTEnumDef* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nenum ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    visit(node->identifier, buffer);
    bufferAppend(buffer, " ");
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->constants, buffer, ",\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    if (node->identifier) { //Hoist enum definition
        bufferAppendPrefix(buffer, "\ntypedef enum ");
        bufferAppendPrefixView(buffer, &node->identifier->token.view);
        bufferAppendPrefix(buffer, " ");
        bufferAppendPrefixView(buffer, &node->identifier->token.view);
        bufferAppendPrefix(buffer, ";");
    }

    FREE(node->constants->array); //Cleanup array allocations
}

void visitFuncDefinition(ASTFuncDef* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    size_t bufStartPos = strlen(buffer->code);
    visit(node->returnType, buffer);
    bufferAppend(buffer, " ");
    visit(node->identifier, buffer);
    bufferAppend(buffer, "(");
    visitCompound(node->arguments, buffer, ", ", true);
    bufferAppend(buffer, ")");
    size_t bufEndPos = strlen(buffer->code);
    bufferAppend(buffer, " {\n");

    bufferIndent(buffer);
    visitCompound(node->statements, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");

    size_t bufSize = bufEndPos - bufStartPos; //Build function definition to hoist
    char* hoist = calloc(bufSize + 2, sizeof(char)); //TODO replace with view
    strncpy(hoist, buffer->code+bufStartPos, bufSize);
    hoist[bufSize] = ';';
    hoist[bufSize + 1] = '\0';
    bufferAppendPrefix(buffer, "\n");
    bufferAppendPrefix(buffer, hoist);

    FREE(node->arguments->array); //Cleanup array allocations
    FREE(node->statements->array);
}

void visitUnionDefinition(ASTUnionDef* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "\nunion ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    if (node->identifier) {
        visit(node->identifier, buffer);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->members, buffer, ";\n", false);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    if (node->identifier) { //Hoist union definition
        bufferAppendPrefix(buffer, "\ntypedef union ");
        bufferAppendPrefixView(buffer, &node->identifier->token.view);
        bufferAppendPrefix(buffer, " ");
        bufferAppendPrefixView(buffer, &node->identifier->token.view);
        bufferAppendPrefix(buffer, ";");
    }

    FREE(node->members->array); //Cleanup array allocations
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
}

void visitReturn(ASTExpr* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visitNode(node->expr, buffer);
    bufferAppend(buffer, ";");
}

void visitImport(AST* node, OutputBuffer* buffer) {
    if (node->token.view.start[0] == '<' || node->token.view.start[0] == '"') { //C Import
        bufferAppendPrefix(buffer, "#include ");
        bufferAppendPrefixView(buffer, &node->token.view);
        bufferAppendPrefix(buffer, "\n");
    } else { //Lava import
        bufferAppendPrefix(buffer, "#include \"");
        bufferAppendPrefixView(buffer, &node->token.view);
        bufferAppendPrefix(buffer, ".h\"\n");
    }
}

void visitBinop(ASTBinop* node, OutputBuffer* buffer) {
    visit(node->left, buffer);
    bufferAppend(buffer, " ");
    bufferAppendView(buffer, &node->op.view);
    bufferAppend(buffer, " ");
    visit(node->right, buffer);
}

void visitAssign(ASTAssign* node, OutputBuffer* buffer) {
    visit(node->left, buffer);
    bufferAppend(buffer, " = ");
    visit(node->right, buffer);
}

void visitInteger(ASTLiteral* node, OutputBuffer* buffer) {
    char str[256] = "";
    snprintf(str, sizeof(str), "%zu", node->value);
    bufferAppend(buffer, str);
}

void visitFuncVar(ASTFuncDef* node, OutputBuffer* buffer) {
    visit(node->returnType, buffer);
    bufferAppend(buffer, " (*");
    visit(node->identifier, buffer);
    bufferAppend(buffer, ")(");
    //Function argument types
    visitCompound(node->arguments, buffer, ", ", true);
    bufferAppend(buffer, ")");

    FREE(node->arguments->array); //Cleanup array allocations

    if (!(node->base.flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visit(AST* node, OutputBuffer* buffer) {
    if (node == NULL) return;
    switch (node->type) {
        case AST_COMP: visitCompound((ASTComp*) node, buffer, "\n", false); break;
        case AST_TYPE: visitDataType(node, buffer); break;
        case AST_ID: case AST_VALUE: visitNode(node, buffer); break;
        case AST_VAR: visitVarDefinition((ASTVarDef*) node, buffer); break;
        case AST_STRUCT: visitStructDefinition((ASTStructDef*) node, buffer); break;
        case AST_ENUM: visitEnumDefinition((ASTEnumDef*) node, buffer); break;
        case AST_FUNC: visitFuncDefinition((ASTFuncDef*) node, buffer); break;
        case AST_UNION: visitUnionDefinition((ASTUnionDef*) node, buffer); break;
        case AST_C: visitCStatement(node, buffer); break;
        case AST_RETURN: visitReturn((ASTExpr*) node, buffer); break;
        case AST_IMPORT: visitImport(node, buffer); break;
        case AST_BINOP: visitBinop((ASTBinop*) node, buffer); break;
        case AST_ASSIGN: visitAssign((ASTAssign*) node, buffer); break;
        case AST_INTEGER: visitInteger((ASTLiteral*) node, buffer); break;
        case AST_FUNC_VAR: visitFuncVar((ASTFuncDef*) node, buffer); break;
        default: PANIC("Unhandled AST: %s %s", AST_NAMES[node->type], node->token.type != TOKEN_NONE_ ? TOKEN_NAMES[node->token.type] : "");
    }
}

OutputBuffer* generateC(ASTComp* root) {
    OutputBuffer* buffer = bufferInit();
    bufferAppend(buffer, "\n\n"); //Initial \n\n to space definitions
    visit((AST*) root, buffer);
    return buffer;
}
#endif //LAVA_CGEN_H