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
        if (i < node->children->len - 1) {
            bufferAppend(buffer, "\n");
        }
    }
}

void visitDataType(AST* node, OutputBuffer* buffer) {
    if (node->token->type == TOKEN_VOID) {
        bufferAppend(buffer, "void");
    } else if (node->token->type == TOKEN_INT) {
        bufferAppend(buffer, "int");
    } else if (node->token->type == TOKEN_I8) {
        bufferAppend(buffer, "int8_t");
    } else if (node->token->type == TOKEN_I16) {
        bufferAppend(buffer, "int16_t");
    } else if (node->token->type == TOKEN_I32) {
        bufferAppend(buffer, "int32_t");
    } else if (node->token->type == TOKEN_I64) {
        bufferAppend(buffer, "int64_t");
    } else if (node->token->type == TOKEN_U8) {
        bufferAppend(buffer, "uint8_t");
    } else if (node->token->type == TOKEN_U16) {
        bufferAppend(buffer, "uint16_t");
    } else if (node->token->type == TOKEN_U32) {
        bufferAppend(buffer, "uint32_t");
    } else if (node->token->type == TOKEN_U64) {
        bufferAppend(buffer, "uint64_t");
    } else if (node->token->type == TOKEN_FLOAT || node->token->type == TOKEN_F32) {
        bufferAppend(buffer, "float");
    } else if (node->token->type == TOKEN_F64) {
        bufferAppend(buffer, "double");
    } else if (node->token->type == TOKEN_STRING || node->token->type == TOKEN_CHAR) {
        bufferAppend(buffer, "char");
    } else if (node->token->type == TOKEN_BOOLEAN) {
        bufferAppend(buffer, "bool");
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
    bufferAppend(buffer, ";");
}

void visitTypeDefinition(ASTStructDef* typeDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "\ntypedef struct ");
    visit(typeDef->identifier, buffer);
    bufferAppend(buffer, "_t {\n");
    bufferIndent(buffer);
        visitCompound(typeDef->members, buffer);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n} ");
    visit(typeDef->identifier, buffer);
    bufferAppend(buffer, ";");
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
        visitCompound(funcDef->statements, buffer);
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n}");
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
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
        visitCompound((ASTCompound*) node, buffer);
    } else if (node->astType == AST_DATA_TYPE) {
        visitDataType(node, buffer);
    } else if (node->astType == AST_IDENTIFIER) {
        visitNode(node, buffer);
    } else if (node->astType == AST_VAR_VALUE) {
        visitNode(node, buffer);
    }

    else if (node->astType == AST_VAR_DEF) {
        visitVarDefinition((ASTVarDef*) node, buffer);
    } else if (node->astType == AST_STRUCT_DEF) {
        visitTypeDefinition((ASTStructDef*) node, buffer);
    } else if (node->astType == AST_FUNC_DEF) {
        visitFuncDefinition((ASTFuncDef*) node, buffer);
    }

    else if (node->astType == AST_C_STATEMENT) {
        visitCStatement(node, buffer);
    } else if (node->astType == AST_RETURN) {
        visitReturn(node, buffer);
    } else if (node->astType == AST_IMPORT) {
        visitImport(node, buffer);
    }

    else {
        PANIC("Unhandled AST: %s %s", AST_NAMES[node->astType], node->token->type != TOKEN_NONE ? TOKEN_NAMES[node->token->type] : "");
    }
}

OutputBuffer* generateC(AST* root) {
    OutputBuffer* buffer = bufferInit();
    //TODO create some sort of bootstrap
    bufferAddImport(buffer, "<stdbool.h>");
    bufferAddImport(buffer, "<stdint.h>");
    visit(root, buffer);
    return buffer;
}
#endif //LAVA_CGEN_H