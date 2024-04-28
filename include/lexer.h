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
    StrView view;
} Lexer;

Lexer* lexerInit(char* filepath, char* contents) {
    Lexer* lexer = RALLOC(1, sizeof(Lexer));
    lexer->filepath = filepath;
    lexer->contents = contents;
    lexer->pos = 0;
    lexer->len = strlen(contents);
    lexer->line = 1;
    lexer->col = 0;
    lexer->cur = contents[lexer->pos];
    lexer->view.start = contents;
    lexer->view.len = 0;
    return lexer;
}

char setLexerPos(Lexer* lexer) {
    return (lexer->cur = lexer->contents[++(lexer->pos)]);
}

char advance(Lexer* lexer) {
    lexer->view.len++;
    return setLexerPos(lexer);
}

char advanceFor(Lexer* lexer, size_t steps) {
    for (int i = 0; i < steps; ++i) {
        advance(lexer);
    }
    return lexer->cur;
}

char advanceNoView(Lexer* lexer) {
    return setLexerPos(lexer);
}

char peek(Lexer* lexer, size_t offset) {
    return lexer->pos + offset >= lexer->len ? '\0' : lexer->contents[lexer->pos + offset];
}

void resetView(Lexer* lexer) {
    lexer->view.start = &lexer->contents[lexer->pos];
    lexer->view.len = 0;
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

void advanceUntilChar(Lexer* lexer, char target, bool includeChar) {
    while (true) { //Consume characters until cur == includeChar
        if (lexer->cur == target) {
            if (includeChar) advance(lexer);
            break;
        }
        advance(lexer);
    }
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
    //TODO crashes arrayFree(lines);
}

#define ERROR(MSG, ...) \
size_t start = findStartOfErrorSnippet(lexer); \
printf("%s:%zu,%zu: ", lexer->filepath, lexer->line, lexer->pos - lexer->col); \
LAVA(MSG, "Internal Error: ", __VA_ARGS__) \
printSyntaxErrorLocation(lexer, start); \
exit(EXIT_FAILURE); \

Token* lexNextDigit(Lexer* lexer) {
    int type = TOKEN_INTEGER_VALUE;
    while (isdigit(lexer->cur) || lexer->cur == '.' || lexer->cur == '_') {
        if (lexer->cur == '.') {
            type = TOKEN_FLOAT_VALUE;
        }
//        if (lexer->cur != '_') {
            //TODO
//        }
        advance(lexer);
    }
    if (lexer->cur == '.' || lexer->cur == '_') { //No trialing . or _
        PANIC("Value: %s is invalid for type %s", viewToStr(&lexer->view), TOKEN_NAMES[type]);
    }
    return tokenInit(type, &lexer->view, DATA_VALUE);
}

size_t lexDataType(Lexer* lexer, TokenFlag type) {
    size_t flags = DATA_TYPE | type;
    if (lexer->cur == '*') {
        advance(lexer);
        flags |= VAR_POINTER;
    } else if (type & VAR_STR) {
        flags |= VAR_POINTER;
    } else if (lexer->cur == '[') {
        advanceFor(lexer, 2);
        flags |= VAR_ARRAY;
    }
    return flags;
}

Token* lexNextCStatement(Lexer* lexer, char endChar) {
    resetView(lexer); //Reset to skip the c. prefix
    advanceUntilChar(lexer, endChar, true); //Consume characters until EOS (;)
    return tokenInit(TOKEN_C_STATEMENT, &lexer->view, 0);
}

Token* lexNextCBlock(Lexer* lexer) {
    advance(lexer); //Skip {
    skipWhitespace(lexer);
    printf("cur: %c\n", lexer->cur);
    return lexNextCStatement(lexer, '}');
}

Token* lexNextImport(Lexer* lexer) {
    skipWhitespace(lexer); //Skip any whitespace after import keyword
    resetView(lexer); //Reset current view to start of import value
    advanceUntilChar(lexer, ';', false); //Consume characters until EOS (;)
    return tokenInit(TOKEN_IMPORT, &lexer->view, 0);
}

Token* lexNextIdentifier(Lexer* lexer) {
    while (isalnum(lexer->cur)) {
        advance(lexer);
    }
    if (viewStrCmp(&lexer->view, "null")) {
        return tokenInit(TOKEN_NULL, strToView("NULL"), 0);
    } else if (viewStrCmp(&lexer->view, "void")) {
        return tokenInit(TOKEN_VOID, &lexer->view, lexDataType(lexer, VAR_VOID));
    } else if (viewStrCmp(&lexer->view, "i8")) {
        return tokenInit(TOKEN_I8, strToView("int8_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "i16")) {
        return tokenInit(TOKEN_I16, strToView("int16_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "int")) {
        return tokenInit(TOKEN_INT, strToView("int"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "i32")) {
        return tokenInit(TOKEN_I32, strToView("int32_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "i64")) {
        return tokenInit(TOKEN_I64, strToView("int64_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "u8")) {
        return tokenInit(TOKEN_U8, strToView("uint8_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "u16")) {
        return tokenInit(TOKEN_U16, strToView("uint16_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "u32")) {
        return tokenInit(TOKEN_U32, strToView("uint32_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "u64")) {
        return tokenInit(TOKEN_U64, strToView("uint64_t"), lexDataType(lexer, VAR_INT));
    } else if  (viewStrCmp(&lexer->view, "usz")) {
        return tokenInit(TOKEN_USZ, strToView("size_t"), lexDataType(lexer, VAR_INT));
    } else if  (viewStrCmp(&lexer->view, "isz")) {
        return tokenInit(TOKEN_ISZ, strToView("ptrdiff_t"), lexDataType(lexer, VAR_INT));
    } else if (viewStrCmp(&lexer->view, "f32")) {
        return tokenInit(TOKEN_F32, strToView("float"), lexDataType(lexer, VAR_FLOAT));
    } else if (viewStrCmp(&lexer->view, "f64")) {
        return tokenInit(TOKEN_F64, strToView("double"), lexDataType(lexer, VAR_FLOAT));
    } else if (viewStrCmp(&lexer->view, "str")) {
        return tokenInit(TOKEN_STRING, strToView("char"), lexDataType(lexer, VAR_STR));
    } else if (viewStrCmp(&lexer->view, "char")) {
        return tokenInit(TOKEN_CHAR, &lexer->view, lexDataType(lexer, VAR_CHAR));
    } else if (viewStrCmp(&lexer->view, "bool")) {
        return tokenInit(TOKEN_BOOLEAN, &lexer->view, lexDataType(lexer, VAR_BOOL));
    } else if (viewStrCmp(&lexer->view, "true") || viewStrCmp(&lexer->view, "false")) {
        return tokenInit(TOKEN_BOOLEAN_VALUE, &lexer->view, DATA_VALUE);
    } else if (viewStrCmp(&lexer->view, "struct")) {
        return tokenInit(TOKEN_STRUCT, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "enum")) {
        return tokenInit(TOKEN_ENUM, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "union")) {
        return tokenInit(TOKEN_UNION, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "flag")) {
        return tokenInit(TOKEN_FLAG, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "packed")) {
        return tokenInit(TOKEN_PACKED, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "if")) {
        return tokenInit(TOKEN_IF, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "else")) {
        return tokenInit(TOKEN_ELSE, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "while")) {
        return tokenInit(TOKEN_WHILE, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "break")) {
        return tokenInit(TOKEN_BREAK, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "for")) {
        return tokenInit(TOKEN_FOR, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "return")) {
        return tokenInit(TOKEN_RETURN, &lexer->view, 0);
    } else if (viewStrCmp(&lexer->view, "import")) {
        return lexNextImport(lexer);
    }
    //Assume if it's not a reserved identifier, it must be a name
    return tokenInit(TOKEN_ID, &lexer->view, 0);
}

Token* lexNextStringOrChar(Lexer* lexer, TokenType type, char enclosingChar) {
    advance(lexer); //Move to next string char
    resetView(lexer); //Reset to skip past this char for the view
    while (lexer->cur != enclosingChar) { //Grow with chars until closing string char
        advance(lexer);
    }
    advanceNoView(lexer); //Advance past closing string char, but don't include it in the view
    return tokenInit(type, &lexer->view, DATA_VALUE);
}

Token* lexNextToken(Lexer* lexer) {
    skipWhitespace(lexer);
    skipComment(lexer);
    resetView(lexer);

    if (lexer->pos >= lexer->len) {
        return tokenInit(TOKEN_EOF, &EMPTY_VIEW, 0);
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
    
    if (lexer->cur == ';') {
        advance(lexer);
        return tokenInit(TOKEN_EOS, &lexer->view, 0);
    } else if (lexer->cur == '=') {
        advance(lexer);
        if (lexer->cur == '=') {
            advance(lexer);
            return tokenInit(TOKEN_EQUALITY, &lexer->view, TYPE_BINOP);
        }
        return tokenInit(TOKEN_ASSIGNMENT, &lexer->view, TYPE_BINOP);
    } else if (lexer->cur == '"') {
        return lexNextStringOrChar(lexer, TOKEN_STRING_VALUE, '"');
    } else if (lexer->cur == '\'') {
        return lexNextStringOrChar(lexer, TOKEN_CHAR_VALUE, '\'');
    } else if (lexer->cur == '/') {
        advance(lexer);
        return tokenInit(TOKEN_DIVIDE, &lexer->view, 0);
    } else if (lexer->cur == '+') {
        advance(lexer);
        if (lexer->cur == '+') {
            advance(lexer);
            return tokenInit(TOKEN_INCREMENT, &lexer->view, TYPE_BINOP);
        }
        return tokenInit(TOKEN_PLUS, &lexer->view, TYPE_BINOP);
    } else if (lexer->cur == '-') {
        advance(lexer);
        if (lexer->cur == '-') {
            advance(lexer);
            return tokenInit(TOKEN_DECREMENT, &lexer->view, TYPE_BINOP);
        }
        return tokenInit(TOKEN_MINUS, &lexer->view, TYPE_BINOP);
    } else if (lexer->cur == '*') {
        advance(lexer);
        return tokenInit(TOKEN_MULTIPLY, &lexer->view, 0);
    } else if (lexer->cur == '<') {
        advance(lexer);
        if (lexer->cur == '=') {
            advance(lexer);
            return tokenInit(TOKEN_LESS_THAN_OR_EQ, &lexer->view, TYPE_BINOP);
        }
        return tokenInit(TOKEN_LESS_THAN, &lexer->view, TYPE_BINOP);
    } else if (lexer->cur == '>') {
        advance(lexer);
        if (lexer->cur == '=') {
            advance(lexer);
            return tokenInit(TOKEN_MORE_THAN_OR_EQ, &lexer->view, TYPE_BINOP);
        }
        return tokenInit(TOKEN_MORE_THAN, &lexer->view, TYPE_BINOP);
    } else if (lexer->cur == '!') {
        advance(lexer);
        if (lexer->cur == '=') {
            advance(lexer);
            return tokenInit(TOKEN_NOT_EQUAL, &lexer->view, 0);
        }
        return tokenInit(TOKEN_NOT, &lexer->view, 0);
    } else if (lexer->cur == '&') {
        advance(lexer);
        if (lexer->cur == '&') {
            advance(lexer);
            return tokenInit(TOKEN_AND, &lexer->view, 0);
        }
        return tokenInit(TOKEN_DEREF, &lexer->view, 0);
    } else if (lexer->cur == '%') {
        advance(lexer);
        return tokenInit(TOKEN_MODULUS, &lexer->view, 0);
    } else if (lexer->cur == '(') {
        advance(lexer);
        return tokenInit(TOKEN_LPAREN, &lexer->view, 0);
    } else if (lexer->cur == ')') {
        advance(lexer);
        return tokenInit(TOKEN_RPAREN, &lexer->view, 0);
    } else if (lexer->cur == ':') {
        advance(lexer);
        return tokenInit(TOKEN_COLON, &lexer->view, 0);
    } else if (lexer->cur == ',') {
        advance(lexer);
        return tokenInit(TOKEN_COMMA, &lexer->view, 0);
    } else if (lexer->cur == '.') {
        advance(lexer);
        return tokenInit(TOKEN_DOT, &lexer->view, 0);
    } else if (lexer->cur == '[') {
        advance(lexer);
        return tokenInit(TOKEN_LBRACKET, &lexer->view, 0);
    } else if (lexer->cur == ']') {
        advance(lexer);
        return tokenInit(TOKEN_RBRACKET, &lexer->view, 0);
    } else if (lexer->cur == '{') {
        advance(lexer);
        return tokenInit(TOKEN_LBRACE, &lexer->view, 0);
    } else if (lexer->cur == '}') {
        advance(lexer);
        return tokenInit(TOKEN_RBRACE, &lexer->view, 0);
    }

    //Finally, return unexpected token is nothing matches
    advance(lexer);
    return tokenInit(TOKEN_UNEXPECTED, &lexer->view, 0);
}
#endif