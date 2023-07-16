#ifndef LAVA_CGEN_H
#define LAVA_CGEN_H

#include <stdio.h>
#include "structures.h"
#include "ast.h"
#include "debug.h"

void visit(AST* node, OutputBuffer* buffer);

void visitNode(AST* node, OutputBuffer* buffer) {
    bufferAppend(buffer, node->token->value);
    astFree(node);
}

void visitCompound(ASTCompound* node, OutputBuffer* buffer, char* delimiter) {
    for (size_t i = 0; i < node->array->len; ++i) {
        bufferAppendIndent(buffer);
        visit((AST*) node->array->elements[i], buffer);
        if (i < node->array->len - 1) {
            bufferAppend(buffer, delimiter);
        }
    }
//    arrayFree(node->array);
    //astFree((AST*) node);
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
    } else if (node->token->type == TOKEN_F32) {
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
    astFree(varDef->dataType);
    astFree((AST*) varDef);
}

void visitStructDefinition(ASTStructDef* structDef, OutputBuffer* buffer) {
    char* hoist = mallocStr("typedef struct ");
    hoist = concatStr(hoist, structDef->identifier->token->value);
    hoist = concatStr(hoist, "_t ");
    hoist = concatStr(hoist, structDef->identifier->token->value);
    hoist = concatStr(hoist, ";");
    bufferAddDef(buffer, hoist);

    bufferAppend(buffer, "\nstruct ");
    visit(structDef->identifier, buffer);
    bufferAppend(buffer, "_t {\n");
    bufferIndent(buffer);
        visitCompound(structDef->members, buffer, "\n");
    bufferUnindent(buffer);
    bufferAppend(buffer, "\n};");

    astFree((AST*) structDef);
}

void visitFuncDefinition(ASTFuncDef* funcDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "\n"); //Append initial new line for space between functions and variables
    size_t bufStartPos = strlen(buffer->code);
    visit(funcDef->returnType, buffer);
    astFree(funcDef->returnType);
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
    astFree((AST*) funcDef->arguments);
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
    char* hoist = calloc(bufSize + 2, sizeof(char));
    strncpy(hoist, buffer->code+bufStartPos, bufSize);
    hoist[bufSize] = ';';
    hoist[bufSize + 1] = '\0';
    bufferAddDef(buffer, hoist);
    astFree((AST*) funcDef);
}

void visitCStatement(AST* node, OutputBuffer* buffer) {
    visitNode(node, buffer);
}

void visitReturn(ASTReturn* returnDef, OutputBuffer* buffer) {
    bufferAppend(buffer, "return ");
    visitNode(returnDef->expression, buffer);
    bufferAppend(buffer, ";");
    astFree((AST*) returnDef);
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
    astFree(node);
}

void visit(AST* node, OutputBuffer* buffer) {
    if (node == NULL) {
        return;
    }

    if (node->astType == AST_COMPOUND) {
        visitCompound((ASTCompound*) node, buffer, "\n");
    } else if (node->astType == AST_DATA_TYPE || node->astType == AST_IDENTIFIER || node->astType == AST_VAR_VALUE) {
        visitDataType(node, buffer);
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
    //TODO create some sort of bootstrap
    bufferAddImport(buffer, "<stdbool.h>");
    bufferAddImport(buffer, "<stdint.h>");
    visit(root, buffer);
    return buffer;
}
#endif //LAVA_CGEN_H