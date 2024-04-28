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

void visitCompound(ASTComp* node, OutputBuffer* buffer, char* delimiter, bool skipLastDelim) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->array->elements[i], buffer);
        //TODO remove AST_IMPORT exclusion somehow
        if ((i < node->array->len - 1 || !skipLastDelim) && ((AST*) node->array->elements[i])->type != AST_IMPORT) {
//            if (pass == GEN) {
                bufferAppend(buffer, delimiter);
//            }
        }
    }
}

void visitDataType(AST* node, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token->view);
    if (node->token->flags & VAR_POINTER) {
        bufferAppend(buffer, "*");
    }
}

void visitVarDefinition(ASTVarDef* node, OutputBuffer* buffer) {
    visit(node->dataType, buffer);
    bufferAppend(buffer, " ");
    visit(node->identifier, buffer);
    if (node->dataType->token->flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (node->expression) {
        bufferAppend(buffer, " = ");
        if (node->dataType->token->type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(node->expression, buffer);
            bufferAppend(buffer, "\"");
        } else if (node->dataType->token->type == TOKEN_CHAR) { //Handle char quotes
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
    bufferPrefix(buffer);
    bufferAppend(buffer, "\ntypedef struct "); //Hoist struct definition
    bufferAppendView(buffer, &node->identifier->token->view);
    bufferAppend(buffer, " ");
    bufferAppendView(buffer, &node->identifier->token->view);
    bufferAppend(buffer, ";\n");

    bufferCode(buffer);
    bufferAppend(buffer, "\nstruct ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    visit(node->identifier, buffer);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
    visitCompound(node->members, buffer, "", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->members->array); //Cleanup array allocations
}

void visitEnumDefinition(ASTEnumDef* node, OutputBuffer* buffer) {
    if (node->identifier) { //Hoist enum definition
        bufferPrefix(buffer);
        bufferAppend(buffer, "\ntypedef enum ");
        bufferAppendView(buffer, &node->identifier->token->view);
        bufferAppend(buffer, " ");
        bufferAppendView(buffer, &node->identifier->token->view);
        bufferAppend(buffer, ";\n");
    }

    bufferCode(buffer);
    bufferAppend(buffer, "\nenum ");
    if (node->base.flags & PACKED_DATA) {
        bufferAppend(buffer, "__attribute__((__packed__)) ");
    }
    if (node->identifier) {
        visit(node->identifier, buffer);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->constants, buffer, ",\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->constants->array); //Cleanup array allocations
}

void visitFuncDefinition(ASTFuncDef* node, OutputBuffer* buffer) {
    bool isStructFunc = node->base.flags & STRUCT_FUNC;
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables

    if (isStructFunc == false) {
        bufferPrefix(buffer);
        bufferAppend(buffer, "\n");
        visit(node->returnType, buffer);
        bufferAppend(buffer, " ");
        if (node->structIden != NULL) {
            bufferAppend(buffer, node->structIden);
            bufferAppend(buffer, "_");
        }
        visit(node->identifier, buffer);
        bufferAppend(buffer, "(");
        visitCompound(node->arguments, buffer, ", ", true);
        bufferAppend(buffer, ");");
    }

    if (isStructFunc == true) {
        bufferPrefix(buffer);
        bufferAppend(buffer, "\n");
    } else {
        bufferCode(buffer);
    }
    visit(node->returnType, buffer);
    bufferAppend(buffer, " ");
    if (node->structIden != NULL) {
        bufferAppend(buffer, node->structIden);
        bufferAppend(buffer, "_");
    }
    visit(node->identifier, buffer);
    bufferAppend(buffer, "(");
    visitCompound(node->arguments, buffer, ", ", true);
    bufferAppend(buffer, ")");
    bufferAppend(buffer, " {\n");

    bufferIndent(buffer);
    visitCompound(node->statements, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");
    bufferCode(buffer);

    FREE(node->arguments->array); //Cleanup array allocations
    FREE(node->statements->array);
}

void visitUnionDefinition(ASTUnionDef* node, OutputBuffer* buffer) {
    if (node->identifier) { //Hoist union definition
        bufferPrefix(buffer);
        bufferAppend(buffer, "\ntypedef union ");
        bufferAppendView(buffer, &node->identifier->token->view);
        bufferAppend(buffer, " ");
        bufferAppendView(buffer, &node->identifier->token->view);
        bufferAppend(buffer, ";");
    }

    bufferCode(buffer);
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

    FREE(node->members->array); //Cleanup array allocations
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
}

void visitReturn(ASTExpr* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visit(node->expr, buffer);
    bufferAppend(buffer, ";");
}

void visitImport(AST* node, OutputBuffer* buffer) {
    bufferPrefix(buffer);
    if (node->token->view.start[0] == '<' || node->token->view.start[0] == '"') { //C Import
        bufferAppend(buffer, "#include ");
        bufferAppendView(buffer, &node->token->view);
        bufferAppend(buffer, "\n");
    } else { //Lava import
        bufferAppend(buffer, "#include \"");
        bufferAppendView(buffer, &node->token->view);
        bufferAppend(buffer, ".h\"\n");
    }
    bufferCode(buffer);
}

void visitBinop(ASTBinop* node, OutputBuffer* buffer) {
//    bufferAppend(buffer, "(");
    visit(node->left, buffer);
    bufferAppend(buffer, " ");
    bufferAppendView(buffer, &node->op->view);
    bufferAppend(buffer, " ");
    visit(node->right, buffer);
//    bufferAppend(buffer, ")");
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

void visitIf(ASTIf* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "if (");
    visit(node->expr, buffer);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visitWhile(ASTWhile* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "while (");
    visit(node->expr, buffer);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visitExpr(ASTExpr* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "(");
    visit(node->expr, buffer);
    bufferAppend(buffer, ")");
}

void visitFuncCall(ASTFuncCall* node, OutputBuffer* buffer) {
    if (node->structIden != NULL) {
        bufferAppend(buffer, node->structIden);
        bufferAppend(buffer, "_");
    }
    visit(node->identifier, buffer);
    bufferAppend(buffer, "(");
    visitCompound(node->expressions, buffer, ",", true);
    bufferAppend(buffer, ")");
}

void visitStructInit(ASTStructInit* node, OutputBuffer* buffer) {
    visit(node->structDef->identifier, buffer);
    bufferAppend(buffer, " ");
    visit(node->identifier, buffer);
    bufferAppend(buffer, " = {");
    visitCompound(node->expressions, buffer, ", ", true);
    bufferAppend(buffer, "};");
    FREE(node->expressions->array);
}

void visitStructMemberRef(ASTStructMemberRef* node, OutputBuffer* buffer) {
    visit(node->varIden, buffer);
    bufferAppend(buffer, ".");
    visit(node->memberIden, buffer);
}

//TODO this should use visitNode, but no semi gets printed
void visitBreak(ASTBreak* node, OutputBuffer* buffer) {
    bufferAppend(buffer, "break;");
}

void visitUnary(ASTUnary* node, OutputBuffer* buffer) {
    if (node->base.flags & UNARY_LEFT) {
        visit(node->expression, buffer);
        bufferAppendView(buffer, &node->op->view);
        bufferAppend(buffer, ";");
    } else {
        bufferAppendView(buffer, &node->op->view);
        visit(node->expression, buffer);
    }
}

void visit(AST* node, OutputBuffer* buffer) {
    if (node == NULL) return;
    switch (node->type) {
        case AST_COMP: visitCompound((ASTComp*) node, buffer, "\n", false); break;
        case AST_TYPE: visitDataType(node, buffer); break;
        case AST_ID: visitNode(node, buffer); break;
        case AST_VALUE: visitNode(node, buffer); break;
        case AST_VAR: visitVarDefinition((ASTVarDef*) node, buffer); break;
        case AST_STRUCT: visitStructDefinition((ASTStructDef*) node, buffer); break;
        case AST_ENUM: visitEnumDefinition((ASTEnumDef*) node, buffer); break;
        case AST_FUNC: visitFuncDefinition((ASTFuncDef*) node, buffer); break;
        case AST_UNION: visitUnionDefinition((ASTUnionDef*) node, buffer); break;
        case AST_C: visitCStatement(node, buffer); break;
        case AST_RETURN: visitReturn((ASTExpr*) node, buffer); break;
        case AST_IMPORT: visitImport(node, buffer); break;
        case AST_BINOP: visitBinop((ASTBinop*) node, buffer); break;
        case AST_UNARY: visitUnary((ASTUnary*) node, buffer); break;
        case AST_ASSIGN: visitAssign((ASTAssign*) node, buffer); break;
        case AST_INTEGER: visitInteger((ASTLiteral*) node, buffer); break;
        case AST_FUNC_VAR: visitFuncVar((ASTFuncDef*) node, buffer); break;
        case AST_IF: visitIf((ASTIf*) node, buffer); break;
        case AST_WHILE: visitWhile((ASTWhile*) node, buffer); break;
        case AST_BREAK: visitBreak((ASTBreak*) node, buffer); break;
        case AST_EXPR: visitExpr((ASTExpr*) node, buffer); break;
        case AST_FUNC_CALL: visitFuncCall((ASTFuncCall*) node, buffer); break;
        case AST_STRUCT_INIT: visitStructInit((ASTStructInit*) node, buffer); break;
        case AST_STRUCT_MEMBER_REF: visitStructMemberRef((ASTStructMemberRef*) node, buffer); break;
        default: PANIC("Unhandled AST: %s %s", AST_NAMES[node->type], node->token->type != TOKEN_NONE_ ? TOKEN_NAMES[node->token->type] : "");
    }
}

void generateC(ASTComp* root, char* prefix, char* code) {
    FILE *fpPrefix = fopen(prefix, "w");
    FILE *fpCode = fopen(code, "w");
    OutputBuffer* buffer = bufferInit(fpPrefix, fpCode);
    bufferPrefix(buffer);
    bufferAppend(buffer, "#include <stdbool.h>\n");
    bufferCode(buffer);
    bufferAppend(buffer, "#include \"output.h\"\n\n");
    visit((AST*) root, buffer);
    bufferFree(buffer);
    fclose(fpPrefix);
    fclose(fpCode);
}
#endif //LAVA_CGEN_H