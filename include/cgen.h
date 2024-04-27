#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

typedef enum GEN_PASS {
    DEF,
    GEN,
} GEN_PASS;

void visit(AST* node, OutputBuffer* buffer, GEN_PASS pass);

void visitNode(AST* node, OutputBuffer* buffer, GEN_PASS pass) {
    bufferAppendView(buffer, &node->token->view);
}

void visitCompound(ASTComp* node, OutputBuffer* buffer, char* delimiter, bool skipLastDelim, GEN_PASS pass) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->array->elements[i], buffer, pass);
        //TODO remove AST_IMPORT exclusion somehow
        if ((i < node->array->len - 1 || !skipLastDelim) && ((AST*) node->array->elements[i])->type != AST_IMPORT) {
            if (pass == GEN) {
                bufferAppend(buffer, delimiter);
            }
        }
    }
}

void visitDataType(AST* node, OutputBuffer* buffer, GEN_PASS pass) {
    bufferAppendView(buffer, &node->token->view);
    if (node->token->flags & VAR_POINTER) {
        bufferAppend(buffer, "*");
    }
}

void visitVarDefinition(ASTVarDef* node, OutputBuffer* buffer, GEN_PASS pass) {
    visit(node->dataType, buffer, pass);
    bufferAppend(buffer, " ");
    visit(node->identifier, buffer, pass);
    if (node->dataType->token->flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (node->expression) {
        bufferAppend(buffer, " = ");
        if (node->dataType->token->type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(node->expression, buffer, pass);
            bufferAppend(buffer, "\"");
        } else if (node->dataType->token->type == TOKEN_CHAR) { //Handle char quotes
            bufferAppend(buffer, "\'");
            visit(node->expression, buffer, pass);
            bufferAppend(buffer, "\'");
        } else {
            visit(node->expression, buffer, pass);
        }
    }
    if (!(node->base.flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visitStructDefinition(ASTStructDef* node, OutputBuffer* buffer, GEN_PASS pass) {
    if (pass == DEF) {
        bufferAppend(buffer, "\ntypedef struct "); //Hoist struct definition
        bufferAppendView(buffer, &node->identifier->token->view);
        bufferAppend(buffer, " ");
        bufferAppendView(buffer, &node->identifier->token->view);
        bufferAppend(buffer, ";");
        return;
    }

    bufferAppend(buffer, "\nstruct ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    visit(node->identifier, buffer, pass);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
    visitCompound(node->members, buffer, "\n", true, pass);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->members->array); //Cleanup array allocations
}

void visitEnumDefinition(ASTEnumDef* node, OutputBuffer* buffer, GEN_PASS pass) {
    if (pass == DEF) {
        if (node->identifier) { //Hoist enum definition
            bufferAppend(buffer, "\ntypedef enum ");
            bufferAppendView(buffer, &node->identifier->token->view);
            bufferAppend(buffer, " ");
            bufferAppendView(buffer, &node->identifier->token->view);
            bufferAppend(buffer, ";");
        }
        return;
    }

    bufferAppend(buffer, "\nenum ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    if (node->identifier) {
        visit(node->identifier, buffer, pass);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->constants, buffer, ",\n", true, pass);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->constants->array); //Cleanup array allocations
}

void visitFuncDefinition(ASTFuncDef* node, OutputBuffer* buffer, GEN_PASS pass) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    if (pass == DEF) {
        visit(node->returnType, buffer, pass);
        bufferAppend(buffer, " ");
        visit(node->identifier, buffer, pass);
        bufferAppend(buffer, "(");
        visitCompound(node->arguments, buffer, ", ", true, pass);
        bufferAppend(buffer, ");");
        return;
    }

    visit(node->returnType, buffer, pass);
    bufferAppend(buffer, " ");
    visit(node->identifier, buffer, pass);
    bufferAppend(buffer, "(");
    visitCompound(node->arguments, buffer, ", ", true, pass);
    bufferAppend(buffer, ")");
    bufferAppend(buffer, " {\n");

    bufferIndent(buffer);
    visitCompound(node->statements, buffer, "\n", true, pass);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");

    FREE(node->arguments->array); //Cleanup array allocations
    FREE(node->statements->array);
}

void visitUnionDefinition(ASTUnionDef* node, OutputBuffer* buffer, GEN_PASS pass) {
    if (pass == DEF) {
        if (node->identifier) { //Hoist union definition
            bufferAppend(buffer, "\ntypedef union ");
            bufferAppendView(buffer, &node->identifier->token->view);
            bufferAppend(buffer, " ");
            bufferAppendView(buffer, &node->identifier->token->view);
            bufferAppend(buffer, ";");
        }
        return;
    }

    bufferAppend(buffer, "\nunion ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    if (node->identifier) {
        visit(node->identifier, buffer, pass);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->members, buffer, ";\n", false, pass);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->members->array); //Cleanup array allocations
}

void visitCStatement(AST* node, OutputBuffer* buffer, GEN_PASS pass) {
    visitNode(node, buffer, pass);
}

void visitReturn(ASTExpr* node, OutputBuffer* buffer, GEN_PASS pass) {
    bufferAppend(buffer, "return ");
    visitNode(node->expr, buffer, pass);
    bufferAppend(buffer, ";");
}

void visitImport(AST* node, OutputBuffer* buffer, GEN_PASS pass) {
    if (pass == DEF) {
        if (node->token->view.start[0] == '<' || node->token->view.start[0] == '"') { //C Import
            bufferAppend(buffer, "#include ");
            bufferAppendView(buffer, &node->token->view);
            bufferAppend(buffer, "\n");
        } else { //Lava import
            bufferAppend(buffer, "#include \"");
            bufferAppendView(buffer, &node->token->view);
            bufferAppend(buffer, ".h\"\n");
        }
        return;
    }
}

void visitBinop(ASTBinop* node, OutputBuffer* buffer, GEN_PASS pass) {
    visit(node->left, buffer, pass);
    bufferAppend(buffer, " ");
    bufferAppendView(buffer, &node->op->view);
    bufferAppend(buffer, " ");
    visit(node->right, buffer, pass);
}

void visitAssign(ASTAssign* node, OutputBuffer* buffer, GEN_PASS pass) {
    visit(node->left, buffer, pass);
    bufferAppend(buffer, " = ");
    visit(node->right, buffer, pass);
}

void visitInteger(ASTLiteral* node, OutputBuffer* buffer, GEN_PASS pass) {
    char str[256] = "";
    snprintf(str, sizeof(str), "%zu", node->value);
    bufferAppend(buffer, str);
}

void visitFuncVar(ASTFuncDef* node, OutputBuffer* buffer, GEN_PASS pass) {
    visit(node->returnType, buffer, pass);
    bufferAppend(buffer, " (*");
    visit(node->identifier, buffer, pass);
    bufferAppend(buffer, ")(");
    //Function argument types
    visitCompound(node->arguments, buffer, ", ", true, pass);
    bufferAppend(buffer, ")");

    FREE(node->arguments->array); //Cleanup array allocations

    if (!(node->base.flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visitIf(ASTIf* node, OutputBuffer* buffer, GEN_PASS pass) {
    bufferAppend(buffer, "if (");
    visit(node->expr, buffer, pass);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, buffer, "\n", true, pass);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visitWhile(ASTWhile* node, OutputBuffer* buffer, GEN_PASS pass) {
    bufferAppend(buffer, "while (");
    visit(node->expr, buffer, pass);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, buffer, "", false, pass);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visit(AST* node, OutputBuffer* buffer, GEN_PASS pass) {
    if (node == NULL) return;
    if (pass == DEF) {
        switch(node->type) {
            case AST_COMP: visitCompound((ASTComp*) node, buffer, "\n", false, pass); break;
            case AST_TYPE: visitDataType(node, buffer, pass); break;
            case AST_ID: case AST_VALUE: visitNode(node, buffer, pass); break;
            case AST_FUNC: visitFuncDefinition((ASTFuncDef*) node, buffer, pass); break;
            case AST_IMPORT: visitImport(node, buffer, pass); break;
            default: return;
        }
    } else {
        switch (node->type) {
            case AST_COMP: visitCompound((ASTComp*) node, buffer, "\n", false, pass); break;
            case AST_TYPE: visitDataType(node, buffer, pass); break;
            case AST_ID: case AST_VALUE: visitNode(node, buffer, pass); break;
            case AST_VAR: visitVarDefinition((ASTVarDef*) node, buffer, pass); break;
            case AST_STRUCT: visitStructDefinition((ASTStructDef*) node, buffer, pass); break;
            case AST_ENUM: visitEnumDefinition((ASTEnumDef*) node, buffer, pass); break;
            case AST_FUNC: visitFuncDefinition((ASTFuncDef*) node, buffer, pass); break;
            case AST_UNION: visitUnionDefinition((ASTUnionDef*) node, buffer, pass); break;
            case AST_C: visitCStatement(node, buffer, pass); break;
            case AST_RETURN: visitReturn((ASTExpr*) node, buffer, pass); break;
            case AST_IMPORT: visitImport(node, buffer, pass); break;
            case AST_BINOP: visitBinop((ASTBinop*) node, buffer, pass); break;
            case AST_ASSIGN: visitAssign((ASTAssign*) node, buffer, pass); break;
            case AST_INTEGER: visitInteger((ASTLiteral*) node, buffer, pass); break;
            case AST_FUNC_VAR: visitFuncVar((ASTFuncDef*) node, buffer, pass); break;
            case AST_IF: visitIf((ASTIf*) node, buffer, pass); break;
            case AST_WHILE: visitWhile((ASTWhile*) node, buffer, pass); break;
            default: PANIC("Unhandled AST: %s %s", AST_NAMES[node->type], node->token->type != TOKEN_NONE_ ? TOKEN_NAMES[node->token->type] : "");
        }
    }
}

void generateC(ASTComp* root, char* filepath) {
    FILE *fp = fopen(filepath, "w");
    OutputBuffer* buffer = bufferInit(fp);
    visit((AST*) root, buffer, DEF);
    bufferAppend(buffer, "\n\n"); //Initial \n\n to space definitions
    visit((AST*) root, buffer, GEN);
    bufferFree(buffer);
}
#endif //LAVA_CGEN_H