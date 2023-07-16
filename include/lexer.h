#ifndef LAVA_LEXER_H
#define LAVA_LEXER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "token.h"
#include "structures.h"
#include "debug.h"

typedef struct Lexer {
    char* filepath;
    char* contents;
    size_t pos;
    size_t len;
    size_t line;
    size_t col; //Only set once error thrown. Position of lava col within contents
    char cur;
} Lexer;

Lexer* lexerInit(char* filepath, char* contents) {
    Lexer* lexer = CALLOC(1, sizeof(Lexer));
    lexer->filepath = filepath;
    lexer->contents = contents;
    lexer->pos = 0;
    lexer->len = strlen(lexer->contents);
    lexer->line = 1;
    lexer->col = 0;
    lexer->cur = lexer->contents[lexer->pos];
    return lexer;
}

void lexerFree(Lexer* lexer) {
    FREE(lexer->contents);
    FREE(lexer);
}

char advance(Lexer* lexer) {
    return (lexer->cur = lexer->contents[++(lexer->pos)]);
}

char advanceFor(Lexer* lexer, size_t steps) {
    for (int i = 0; i < steps; ++i) {
        advance(lexer);
    }
    return lexer->cur;
}

char peek(Lexer* lexer, size_t offset) {
    return lexer->pos + offset >= lexer->len ? '\0' : lexer->contents[lexer->pos + offset];
}

int isCharWhitespace(char c) {
    return c == ' ' || c == '\n' || c == '\r';
}

int isCurWhitespace(Lexer* lexer) {
    return isCharWhitespace(lexer->cur);
}

void skipWhitespace(Lexer* lexer) {
    while (isCurWhitespace(lexer)) {
        if (lexer->cur == '\n') {
            lexer->line++;
        }
        advance(lexer);
    }
}

void skipComment(Lexer* lexer) {
    while (lexer->cur == '/' && (peek(lexer, 1) == '/' || peek(lexer, 1) == '*')) {
        advance(lexer);
        char end = lexer->cur == '/' ? '\n' : '*'; //Keep track if this comment should end with \n or *
        advance(lexer); //Advance past that starting character and being reading the comment text
        while (lexer->cur != end) {
            advance(lexer);
        }
        if (end == '*') { //Advance past the trailing / of multi line comments
            advance(lexer);
            advance(lexer);
        }
        skipWhitespace(lexer);
    }
}

char* buildBufferUntilChar(Lexer* lexer, char* buffer, char target, bool includeChar) {
    bool shouldLoop = true;
    while (shouldLoop) { //Consume characters until cur == includeChar
        if (lexer->cur == target) {
            if (includeChar) {
                shouldLoop = false;
            } else {
                break;
            }
        }
        buffer = REALLOC(buffer, (strlen(buffer) + 2) * sizeof(char));
        strncat(buffer, lexer->contents + lexer->pos, 1);
        advance(lexer);
    }
    return buffer;
}

size_t findStartOfErrorSnippet(Lexer* lexer) {
    size_t start = 0;
    size_t newLineCount = 0;
    for (size_t i = lexer->pos; i > 0; i--) { //Find starting point
        if (lexer->contents[i] == '\n') {
            newLineCount++;
            if (newLineCount == 1) {
                lexer->col = i + 1; //Start if first \n for getting the lava column
            } else if (newLineCount == 3) {
                return start;
            }
        }
        start = i;
    }
    return start;
}

void printSyntaxErrorLocation(Lexer* lexer, size_t start) {
    size_t endCurrentLine = 0;
    for (size_t i = lexer->pos; i < lexer->len; ++i) {
        if (lexer->contents[i] == '\n') {
            endCurrentLine = i + 1; //+1 to include the \n itself
            break;
        }
    }

    DynArray* lines = arrayInit(sizeof(char *));
    size_t lineStart = start;
    for (size_t i = start; i < endCurrentLine; i++) {
        if (lexer->contents[i] == '\n') {
            size_t lineLen = i - lineStart;
            char* line = MALLOC(lineLen * sizeof(char) + 1);
            strncpy(line, lexer->contents+lineStart, lineLen);
            line[lineLen + 1] = '\0';
            arrayAppend(lines, line);
            lineStart = i + 1; //+1 so the next start moves past the \n
        }
    }
    printf("\n");
    for (int i = 0; i < lines->len; ++i) {
        printf("> %s\n", (char*) lines->elements[i]);
    }
    printf("> ");
    for (int i = 0; i < lexer->pos - lexer->col - 1; ++i) {
        putchar('-');
    }
    printf("^\n");
    arrayFree(lines);
}

#define ERROR(MSG, ...) \
size_t start = findStartOfErrorSnippet(lexer); \
printf("%s:%zu,%zu: ", lexer->filepath, lexer->line, lexer->pos - lexer->col); \
LAVA(MSG, "Internal Error: ", __VA_ARGS__) \
printSyntaxErrorLocation(lexer, start); \
exit(EXIT_FAILURE); \

Token* lexNextDigit(Lexer* lexer) {
    char* buffer = charToStr(lexer->cur);
    int type = TOKEN_INTEGER_VALUE;

    advance(lexer);
    while (isdigit(lexer->cur) || lexer->cur == '.' || lexer->cur == '_') {
        if (lexer->cur == '.') {
            type = TOKEN_FLOAT_VALUE;
        }
        if (lexer->cur != '_') {
            buffer = REALLOC(buffer, (strlen(buffer) + 2) * sizeof(char));
            strncat(buffer, lexer->contents + lexer->pos, 1);
        }
        advance(lexer);
    }
    if (buffer[strlen(buffer) - 1] == '.') {
        PANIC("Value: %s is invalid for type %s", buffer, TOKEN_NAMES[type]);
    }

    return tokenInit(type, buffer, 0);
}

Token* lexNextVarType(Lexer* lexer, TokenType tokenType, char* buffer, TokenFlag type) {
    size_t flags = VAR_TYPE | type;
    if (lexer->cur == '*' || tokenType == TOKEN_STRING) {
        advance(lexer);
        flags |= VAR_POINTER;
    } else if (lexer->cur == '[') {
        advanceFor(lexer, 2);
        flags |= VAR_ARRAY;
    }
    return tokenInit(tokenType, buffer, flags);
}

Token* lexNextCStatement(Lexer* lexer, char endChar) {
    char* buffer = charToStr(lexer->cur); //Buffer initialized with first C char
    advance(lexer); //Advance past first comment char, since it is now in the initial buffer
    buffer = buildBufferUntilChar(lexer, buffer, endChar, true); //Consume characters until EOS (;)
    advance(lexer); //Advance past closing C token character
    return tokenInit(TOKEN_C_STATEMENT, buffer, 0);
}

Token* lexNextCBlock(Lexer* lexer) {
    advance(lexer); //Skip {
    skipWhitespace(lexer);
    printf("cur: %c\n", lexer->cur);
    return lexNextCStatement(lexer, '}');
}

Token* lexNextImport(Lexer* lexer) {
    skipWhitespace(lexer);
    char* buffer = charToStr(lexer->cur);
    advance(lexer);
    buffer = buildBufferUntilChar(lexer, buffer, ';', false); //Consume characters until EOS (;)
    return tokenInit(TOKEN_IMPORT, buffer, 0);
}

Token* lexNextIdentifier(Lexer* lexer) {
    char* buffer = charToStr(lexer->cur);
    advance(lexer);
    while (isalnum(lexer->cur)) {
        buffer = REALLOC(buffer, (strlen(buffer) + 2) * sizeof(char));
        strncat(buffer, lexer->contents + lexer->pos, 1);
        advance(lexer);
    }

    //TODO pass C types directly here instead of redoing work in cgen
    if (strcmp(buffer, "null") == 0) {
        return tokenInit(TOKEN_NULL, buffer, 0);
    } else if (strcmp(buffer, "void") == 0) {
        return tokenInit(TOKEN_VOID, buffer, VAR_TYPE);
    } else if (strcmp(buffer, "i8") == 0) {
        return lexNextVarType(lexer, TOKEN_I8, buffer, VAR_INT);
    } else if (strcmp(buffer, "i16") == 0) {
        return lexNextVarType(lexer, TOKEN_I16, buffer, VAR_INT);
    } else if (strcmp(buffer, "int") == 0) {
        return lexNextVarType(lexer, TOKEN_INT, buffer, VAR_INT);
    } else if (strcmp(buffer, "i32") == 0) {
        return lexNextVarType(lexer, TOKEN_I32, buffer, VAR_INT);
    } else if (strcmp(buffer, "i64") == 0) {
        return lexNextVarType(lexer, TOKEN_I64, buffer, VAR_INT);
    } else if (strcmp(buffer, "u8") == 0) {
        return lexNextVarType(lexer, TOKEN_U8, buffer, VAR_INT);
    } else if (strcmp(buffer, "u16") == 0) {
        return lexNextVarType(lexer, TOKEN_U16, buffer, VAR_INT);
    } else if (strcmp(buffer, "u32") == 0) {
        return lexNextVarType(lexer, TOKEN_U32, buffer, VAR_INT);
    } else if (strcmp(buffer, "u64") == 0) {
        return lexNextVarType(lexer, TOKEN_U64, buffer, VAR_INT);
    } else if (strcmp(buffer, "f32") == 0 || strcmp(buffer, "float") == 0) {
        return lexNextVarType(lexer, TOKEN_F32, buffer, VAR_FLOAT);
    } else if (strcmp(buffer, "f64") == 0) {
        return lexNextVarType(lexer, TOKEN_F64, buffer, VAR_FLOAT);
    } else if (strcmp(buffer, "str") == 0) {
        return lexNextVarType(lexer, TOKEN_STRING, buffer, VAR_STR);
    } else if (strcmp(buffer, "char") == 0) {
        return lexNextVarType(lexer, TOKEN_CHAR, buffer, VAR_CHAR);
    } else if (strcmp(buffer, "bool") == 0) {
        return lexNextVarType(lexer, TOKEN_BOOLEAN, buffer, VAR_BOOL);
    } else if (strcmp(buffer, "true") == 0 || strcmp(buffer, "false") == 0) {
        return tokenInit(TOKEN_BOOLEAN_VALUE, buffer, 0);
    } else if (strcmp(buffer, "struct") == 0) { //TODO move to lexKeyword
        return tokenInit(TOKEN_STRUCT_DEF, buffer, 0);
    } else if (strcmp(buffer, "enum") == 0) {
        return tokenInit(TOKEN_ENUM_DEF, buffer, 0);
    } else if (strcmp(buffer, "if") == 0) {
        return tokenInit(TOKEN_IF, buffer, 0);
    } else if (strcmp(buffer, "else") == 0) {
        return tokenInit(TOKEN_ELSE, buffer, 0);
    } else if (strcmp(buffer, "while") == 0) {
        return tokenInit(TOKEN_WHILE, buffer, 0);
    } else if (strcmp(buffer, "for") == 0) {
        return tokenInit(TOKEN_FOR, buffer, 0);
    } else if (strcmp(buffer, "return") == 0) {
        return tokenInit(TOKEN_RETURN, buffer, 0);
    } else if (strcmp(buffer, "import") == 0) {
        FREE(buffer); //TODO pass to lexNextImport?
        return lexNextImport(lexer);
    }
    //Assume if it's not a reserved identifier, it must be a name
    return tokenInit(TOKEN_IDENTIFIER, buffer, 0);
}

Token* lexNextStringOrChar(Lexer* lexer, TokenType type, char enclosingChar) {
    char* buffer = charToStr(lexer->cur); //Contains first string char
    advance(lexer); //Move to next string char
    while (lexer->cur != enclosingChar) { //Grow with chars until closing string char
        buffer = REALLOC(buffer, (strlen(buffer) + 2) * sizeof(char));
        strncat(buffer, lexer->contents + lexer->pos, 1);
        advance(lexer);
    }
    advance(lexer); //Advance past closing string char
    return tokenInit(type, buffer, 0);
}

Token* lexNextToken(Lexer* lexer) {
    skipWhitespace(lexer);
    skipComment(lexer);

    if (lexer->pos >= lexer->len) {
        return tokenInit(TOKEN_EOF, mallocStr("EOF"), 0);
    } else if (lexer->cur == 'c' && peek(lexer, 1) == '.') {
        /*if (peek(lexer, 1) == '.') {*/
            advance(lexer);
            advance(lexer); //Advance twice to move past "c." prefix
            return lexNextCStatement(lexer, ';');
        /*} else {
            char next = peek(lexer, 1);
            size_t i = 1;
            while (isCharWhitespace(next)) {
                next = peek(lexer, ++i);
            }
            if (next == '{') {
                for (int j = 0; j < i; ++j) {
                    advance(lexer);
                }
                return lexNextCBlock(lexer);
            }
        }*/
    } else if (isdigit(lexer->cur)) {
        return lexNextDigit(lexer);
    } else if (isalnum(lexer->cur)) {
        return lexNextIdentifier(lexer);
    }

    char* value = charToStr(lexer->cur);
    if (lexer->cur == ';') {
        advance(lexer);
        return tokenInit(TOKEN_EOS, value, 0);
    } else if (lexer->cur == '=') {
        advance(lexer);
        if (lexer->cur == '=') {
            FREE(value);
            advance(lexer);
            return tokenInit(TOKEN_EQUALS, "==", 0);
        } else {
            return tokenInit(TOKEN_ASSIGNMENT, value, 0);
        }
    } else if (lexer->cur == '"') {
        advance(lexer);
        return lexNextStringOrChar(lexer, TOKEN_STRING_VALUE, '"');
    } else if (lexer->cur == '\'') {
        advance(lexer);
        return lexNextStringOrChar(lexer, TOKEN_CHAR_VALUE, '\'');
    } else if (lexer->cur == '/') {
        advance(lexer);
        return tokenInit(TOKEN_DIVISION, value, 0);
    } else if (lexer->cur == '+') {
        advance(lexer);
        return tokenInit(TOKEN_PLUS, value, 0);
    } else if (lexer->cur == '-') {
        advance(lexer);
        return tokenInit(TOKEN_MINUS, value, 0);
    } else if (lexer->cur == '*') {
        advance(lexer);
        return tokenInit(TOKEN_MULTIPLY, value, 0);
    } else if (lexer->cur == '<') {
        advance(lexer);
        return tokenInit(TOKEN_LESS_THAN, value, 0);
    } else if (lexer->cur == '>') {
        advance(lexer);
        return tokenInit(TOKEN_MORE_THAN, value, 0);
    } else if (lexer->cur == '!') {
        advance(lexer);
        return tokenInit(TOKEN_NOT, value, 0);
    } else if (lexer->cur == '(') {
        advance(lexer);
        return tokenInit(TOKEN_LPAREN, value, 0);
    } else if (lexer->cur == ')') {
        advance(lexer);
        return tokenInit(TOKEN_RPAREN, value, 0);
    } else if (lexer->cur == ':') {
        advance(lexer);
        return tokenInit(TOKEN_COLON, value, 0);
    } else if (lexer->cur == ',') {
        advance(lexer);
        return tokenInit(TOKEN_COMMA, value, 0);
    } else if (lexer->cur == '.') {
        advance(lexer);
        return tokenInit(TOKEN_DOT, value, 0);
    } else if (lexer->cur == '[') {
        advance(lexer);
        return tokenInit(TOKEN_LBRACKET, value, 0);
    } else if (lexer->cur == ']') {
        advance(lexer);
        return tokenInit(TOKEN_RBRACKET, value, 0);
    } else if (lexer->cur == '{') {
        advance(lexer);
        return tokenInit(TOKEN_LBRACE, value, 0);
    } else if (lexer->cur == '}') {
        advance(lexer);
        return tokenInit(TOKEN_RBRACE, value, 0);
    }

    //Finally, return unexpected token is nothing matches
    advance(lexer);
    return tokenInit(TOKEN_UNEXPECTED, value, 0);
}
#endif