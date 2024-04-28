#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* node, ASTType parent, OutputBuffer* buffer);

void visitNode(AST* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token->view);
}

void visitCompound(ASTComp* node, ASTType parent, OutputBuffer* buffer, char* delimiter, bool skipLastDelim) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->array->elements[i], parent, buffer);
        //TODO remove AST_IMPORT exclusion somehow
        if ((i < node->array->len - 1 || !skipLastDelim) && ((AST*) node->array->elements[i])->type != AST_IMPORT) {
//            if (pass == GEN) {
                bufferAppend(buffer, delimiter);
//            }
        }
    }
}

void visitDataType(AST* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppendView(buffer, &node->token->view);
    if (node->flags & POINTER_TYPE) {
        bufferAppend(buffer, "*");
    }
}

void visitVarDefinition(ASTVarDef* node, ASTType parent, OutputBuffer* buffer) {
    visit(node->dataType, parent, buffer);
//    if (node->dataType->token->flags & VAR_POINTER) {
//        bufferAppend(buffer, "*");
//    }
    bufferAppend(buffer, " ");
    visit(node->identifier, parent, buffer);
    if (node->dataType->token->flags & VAR_ARRAY) {
        bufferAppend(buffer, "[]");
    }
    if (node->expression) {
        bufferAppend(buffer, " = ");
        if (node->dataType->token->type == TOKEN_STRING) { //Handle string quotes
            bufferAppend(buffer, "\"");
            visit(node->expression, parent, buffer);
            bufferAppend(buffer, "\"");
        } else if (node->dataType->token->type == TOKEN_CHAR) { //Handle char quotes
            bufferAppend(buffer, "\'");
            visit(node->expression, parent, buffer);
            bufferAppend(buffer, "\'");
        } else {
            visit(node->expression, parent, buffer);
        }
    }
    if (!(node->base.flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visitStructDefinition(ASTStructDef* node, ASTType parent, OutputBuffer* buffer) {
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
    visit(node->identifier, parent, buffer);
    bufferAppend(buffer, " {\n");
    bufferIndent(buffer);
    visitCompound(node->members, parent, buffer, "", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->members->array); //Cleanup array allocations
}

void visitEnumDefinition(ASTEnumDef* node, ASTType parent, OutputBuffer* buffer) {
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
        visit(node->identifier, parent, buffer);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->constants, parent, buffer, ",\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->constants->array); //Cleanup array allocations
}

void visitFuncDefinition(ASTFuncDef* node, ASTType parent, OutputBuffer* buffer) {
    bool isStructFunc = node->base.flags & STRUCT_FUNC;
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables

    if (isStructFunc == false) {
        bufferPrefix(buffer);
        bufferAppend(buffer, "\n");
        visit(node->returnType, parent, buffer);
        bufferAppend(buffer, " ");
        if (node->structIden != NULL) {
            bufferAppend(buffer, node->structIden);
            bufferAppend(buffer, "_");
        }
        visit(node->identifier, parent, buffer);
        bufferAppend(buffer, "(");
        visitCompound(node->arguments, parent, buffer, ", ", true);
        bufferAppend(buffer, ");");
    }

    if (isStructFunc == true) {
        bufferPrefix(buffer);
        bufferAppend(buffer, "\n");
    } else {
        bufferCode(buffer);
    }
    visit(node->returnType, parent, buffer);
    bufferAppend(buffer, " ");
    if (node->structIden != NULL) {
        bufferAppend(buffer, node->structIden);
        bufferAppend(buffer, "_");
    }
    visit(node->identifier, parent, buffer);
    bufferAppend(buffer, "(");
    visitCompound(node->arguments, parent, buffer, ", ", true);
    bufferAppend(buffer, ")");
    bufferAppend(buffer, " {\n");

    bufferIndent(buffer);
    visitCompound(node->statements, parent, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");
    bufferCode(buffer);

    FREE(node->arguments->array); //Cleanup array allocations
    FREE(node->statements->array);
}

void visitUnionDefinition(ASTUnionDef* node, ASTType parent, OutputBuffer* buffer) {
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
        visit(node->identifier, parent, buffer);
        bufferAppend(buffer, " ");
    }
    bufferAppend(buffer, "{\n");
    bufferIndent(buffer);
    visitCompound(node->members, parent, buffer, ";\n", false);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    FREE(node->members->array); //Cleanup array allocations
}

void visitCStatement(AST* node, ASTType parent, OutputBuffer* buffer) {
    visitNode(node, parent, buffer);
}

void visitReturn(ASTExpr* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visit(node->expr, parent, buffer);
    bufferAppend(buffer, ";");
}

void visitImport(AST* node, ASTType parent, OutputBuffer* buffer) {
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

void visitBinop(ASTBinop* node, ASTType parent, OutputBuffer* buffer) {
//    bufferAppend(buffer, "(");
    visit(node->left, parent, buffer);
    bufferAppend(buffer, " ");
    bufferAppendView(buffer, &node->op->view);
    bufferAppend(buffer, " ");
    visit(node->right, parent, buffer);
//    bufferAppend(buffer, ")");
}

void visitAssign(ASTAssign* node, ASTType parent, OutputBuffer* buffer) {
    visit(node->left, parent, buffer);
    bufferAppend(buffer, " = ");
    visit(node->right, parent, buffer);
}

void visitInteger(ASTLiteral* node, ASTType parent, OutputBuffer* buffer) {
    char str[256] = "";
    snprintf(str, sizeof(str), "%zu", node->value);
    bufferAppend(buffer, str);
}

void visitFuncVar(ASTFuncDef* node, ASTType parent, OutputBuffer* buffer) {
    visit(node->returnType, parent, buffer);
    bufferAppend(buffer, " (*");
    visit(node->identifier, parent, buffer);
    bufferAppend(buffer, ")(");
    //Function argument types
    visitCompound(node->arguments, parent, buffer, ", ", true);
    bufferAppend(buffer, ")");

    FREE(node->arguments->array); //Cleanup array allocations

    if (!(node->base.flags & ARGUMENT)) {
        bufferAppend(buffer, ";");
    }
}

void visitIf(ASTIf* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "if (");
    visit(node->expr, parent, buffer);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, parent, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visitElse(ASTElse* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "else ");
    if (node->body != NULL) {
        bufferAppend(buffer, "{\n");
        bufferIndent(buffer);
        visitCompound(node->body, parent, buffer, "\n", true);
        bufferUnindent(buffer);
        bufferAppend(buffer, "\n");
        bufferAppendIndent(buffer);
        bufferAppend(buffer, "}");
    }
}

void visitWhile(ASTWhile* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "while (");
    visit(node->expr, parent, buffer);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, parent, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visitFor(ASTFor* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "for (");
    visit(node->definition, parent, buffer);
    bufferAppend(buffer, " ");
    visit(node->condition, parent, buffer);
    bufferAppend(buffer, "; ");
    visit(node->expression, AST_FOR, buffer);
    bufferAppend(buffer, ") {\n");
    bufferIndent(buffer);
    visitCompound(node->body, parent, buffer, "\n", true);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n");
    bufferAppendIndent(buffer);
    bufferAppend(buffer, "}");
    FREE(node->body->array);
}

void visitExpr(ASTExpr* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "(");
    visit(node->expr, parent, buffer);
    bufferAppend(buffer, ")");
}

void visitFuncCall(ASTFuncCall* node, ASTType parent, OutputBuffer* buffer) {
    if (node->structIden != NULL) {
        bufferAppend(buffer, node->structIden);
        bufferAppend(buffer, "_");
    }
    visit(node->identifier, parent, buffer);
    bufferAppend(buffer, "(");
    visitCompound(node->expressions, parent, buffer, ",", true);
    bufferAppend(buffer, ")");
    if (node->base.flags & NON_EXPR_FUNC) {
        bufferAppend(buffer, ";");
    }
}

void visitStructInit(ASTStructInit* node, ASTType parent, OutputBuffer* buffer) {
    visit(node->structDef->identifier, parent, buffer);
    if (node->structDef->base.flags & POINTER_TYPE) {
        bufferAppend(buffer, "*");
    }
    bufferAppend(buffer, " ");
    visit(node->identifier, parent, buffer);
    bufferAppend(buffer, " = {");
    visitCompound(node->expressions, parent, buffer, ", ", true);
    bufferAppend(buffer, "};");
    FREE(node->expressions->array);
}

void visitStructMemberRef(ASTStructMemberRef* node, ASTType parent, OutputBuffer* buffer) {
    visit(node->varIden, parent, buffer);
    bufferAppend(buffer, ".");
    visit(node->memberIden, parent, buffer);
}

//TODO this should use visitNode, but no semi gets printed
void visitBreak(ASTBreak* node, ASTType parent, OutputBuffer* buffer) {
    bufferAppend(buffer, "break;");
}

//TODO visit should have access to the parent node, so we can exclude for loop EOS
void visitUnary(ASTUnary* node, ASTType parent, OutputBuffer* buffer) {
    if (node->base.flags & UNARY_LEFT) {
        visit(node->expression, parent, buffer);
        bufferAppendView(buffer, &node->op->view);
        if (parent != AST_FOR) {
            bufferAppend(buffer, ";");
        }
    } else {
        bufferAppendView(buffer, &node->op->view);
        visit(node->expression, parent, buffer);
    }
}

void visit(AST* node, ASTType parent, OutputBuffer* buffer) {
    if (node == NULL) return;
    switch (node->type) {
        case AST_COMP: visitCompound((ASTComp*) node, parent, buffer, "\n", false); break;
        case AST_TYPE: visitDataType(node, parent, buffer); break;
        case AST_ID: visitNode(node, parent, buffer); break;
        case AST_VALUE: visitNode(node, parent, buffer); break;
        case AST_VAR: visitVarDefinition((ASTVarDef*) node, parent, buffer); break;
        case AST_STRUCT: visitStructDefinition((ASTStructDef*) node, parent, buffer); break;
        case AST_ENUM: visitEnumDefinition((ASTEnumDef*) node, parent, buffer); break;
        case AST_FUNC: visitFuncDefinition((ASTFuncDef*) node, parent, buffer); break;
        case AST_UNION: visitUnionDefinition((ASTUnionDef*) node, parent, buffer); break;
        case AST_C: visitCStatement(node, parent, buffer); break;
        case AST_RETURN: visitReturn((ASTExpr*) node, parent, buffer); break;
        case AST_IMPORT: visitImport(node, parent, buffer); break;
        case AST_BINOP: visitBinop((ASTBinop*) node, parent, buffer); break;
        case AST_UNARY: visitUnary((ASTUnary*) node, parent, buffer); break;
        case AST_ASSIGN: visitAssign((ASTAssign*) node, parent, buffer); break;
        case AST_INTEGER: visitInteger((ASTLiteral*) node, parent, buffer); break;
        case AST_FUNC_VAR: visitFuncVar((ASTFuncDef*) node, parent, buffer); break;
        case AST_IF: visitIf((ASTIf*) node, parent, buffer); break;
        case AST_ELSE: visitElse((ASTElse*) node, parent, buffer); break;
        case AST_WHILE: visitWhile((ASTWhile*) node, parent, buffer); break;
        case AST_FOR: visitFor((ASTFor*) node, parent, buffer); break;
        case AST_BREAK: visitBreak((ASTBreak*) node, parent, buffer); break;
        case AST_EXPR: visitExpr((ASTExpr*) node, parent, buffer); break;
        case AST_FUNC_CALL: visitFuncCall((ASTFuncCall*) node, parent, buffer); break;
        case AST_STRUCT_INIT: visitStructInit((ASTStructInit*) node, parent, buffer); break;
        case AST_STRUCT_MEMBER_REF: visitStructMemberRef((ASTStructMemberRef*) node, parent, buffer); break;
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
    visit((AST*) root, -1, buffer);
    bufferFree(buffer);
    fclose(fpPrefix);
    fclose(fpCode);
}
#endif //LAVA_CGEN_H