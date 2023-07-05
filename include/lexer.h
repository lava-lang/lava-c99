#ifndef LAVA_LEXER_H
#define LAVA_LEXER_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "token.h"
#include "debug.h"
#include "util.h"

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
    Lexer* lexer = malloc(sizeof(Lexer));
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
    free(lexer->contents);
    free(lexer);
}

char advance(Lexer* lexer) {
    return (lexer->cur = lexer->contents[++(lexer->pos)]);
}

char peek(Lexer* lexer, int offset) {
    return lexer->pos + offset >= lexer->len ? '\0' : lexer->contents[lexer->pos + offset];
}

int isWhitespace(Lexer* lexer) {
    return lexer->cur == ' ' || lexer->cur == 10 || lexer->cur == '\r';
}

void skipWhitespace(Lexer* lexer) {
    while (isWhitespace(lexer)) {
        if (lexer->cur == '\n') {
            lexer->line++;
        }
        advance(lexer);
    }
}

char* buildBufferUntilEOS(Lexer* lexer, char* buffer, bool includeEOS) {
    bool shouldLoop = true;
    while (shouldLoop) { //Consume characters until EOS (;)
        if (lexer->cur == ';') {
            if (includeEOS) {
                shouldLoop = false;
            } else {
                break;
            }
        }

        //TODO realloc null check macro
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
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

    List* lines = listInit(sizeof(char*));
    size_t lineStart = start;
    for (size_t i = start; i < endCurrentLine; i++) {
        if (lexer->contents[i] == '\n') {
            size_t lineLen = i - lineStart;
            char* line = malloc(lineLen * sizeof(char) + 1);
            strncpy(line, lexer->contents+lineStart, lineLen);
            line[lineLen + 1] = '\0';
            listAppend(lines, line);
            lineStart = i + 1; //+1 so the next start moves past the \n
        }
    }
    printf("\n");
    for (int i = 0; i < lines->len; ++i) {
        printf("> %s\n", (char*) lines->elements[i]);
    }
    printf ("> ");
    for (int i = 0; i < lexer->pos - lexer->col - 2; ++i) {
        putchar('-');
    }
    printf("^\n");
}

#define ERROR(MSG, ...) \
size_t start = findStartOfErrorSnippet(lexer); \
printf("%s:%zu,%zu: ", lexer->filepath, lexer->line, lexer->pos - lexer->col); \
LAVA(MSG, "Lava Error: ", __VA_ARGS__) \
printSyntaxErrorLocation(lexer, start); \
exit(1); \

Token* lexNextDigit(Lexer* lexer) {
    char* buffer = charToStr(lexer->cur);
    int type = TOKEN_INTEGER_VALUE;

    advance(lexer);
    while (isdigit(lexer->cur) || lexer->cur == '.' || lexer->cur == '_') {
        if (lexer->cur == '.') {
            type = TOKEN_FLOAT_VALUE;
        }
        if (lexer->cur != '_') {
            buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
            strncat(buffer, lexer->contents + lexer->pos, 1);
        }
        advance(lexer);
    }
    if (buffer[strlen(buffer) - 1] == '.') {
        PANIC("Value: %s is invalid for type %s", buffer, TOKEN_NAMES[type]);
    }

    return tokenInit(type, buffer);
}

Token* lexNextVarType(Lexer* lexer, TokenType tokenType, char* buffer, TokenType valueType) {
    bool isPointer = false;
    if (lexer->cur == '*') {
        advance(lexer);
        isPointer = true;
    }
    if (tokenType == TOKEN_STRING) {
        isPointer = true;
    }
    //TODO boolean values...
    //TODO detect arrays
    return tokenVarInit(tokenType, buffer, valueType, isPointer);
}

Token* lexNextCStatement(Lexer* lexer) {
    char* buffer = charToStr(lexer->cur); //Buffer initialized with first C char
    advance(lexer); //Advance past first comment char, since it is now in the initial buffer
    buffer = buildBufferUntilEOS(lexer, buffer, true); //Consume characters until EOS (;)
    advance(lexer); //Advance past closing C token character
    return tokenInit(TOKEN_C_STATEMENT, buffer);
}

Token* lexNextImport(Lexer* lexer) {
    skipWhitespace(lexer);
    char* buffer = charToStr(lexer->cur);
    advance(lexer);
    buffer = buildBufferUntilEOS(lexer, buffer, false); //Consume characters until EOS (;)
    return tokenInit(TOKEN_IMPORT, buffer);
}

Token* lexNextIdentifier(Lexer* lexer) {
    char* buffer = charToStr(lexer->cur);

    advance(lexer);
    while (isalnum(lexer->cur)) {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strncat(buffer, lexer->contents + lexer->pos, 1);
        advance(lexer);
    }

    if (strcmp(buffer, "null") == 0) {
        return tokenInit(TOKEN_NULL, buffer);
    } else if (strcmp(buffer, "void") == 0) {
        return tokenInit(TOKEN_VOID, buffer);
    } else if (strcmp(buffer, "int") == 0) {
        return lexNextVarType(lexer, TOKEN_INT, buffer, TOKEN_INTEGER_VALUE);
    } else if (strcmp(buffer, "i32") == 0) {
        return lexNextVarType(lexer, TOKEN_I32, buffer, TOKEN_INTEGER_VALUE);
    } else if (strcmp(buffer, "i64") == 0) {
        return lexNextVarType(lexer, TOKEN_I64, buffer, TOKEN_INTEGER_VALUE);
    } else if (strcmp(buffer, "float") == 0) {
        return lexNextVarType(lexer, TOKEN_F32, buffer, TOKEN_FLOAT_VALUE);
    } else if (strcmp(buffer, "f32") == 0) {
        return lexNextVarType(lexer, TOKEN_F32, buffer, TOKEN_FLOAT_VALUE);
    } else if (strcmp(buffer, "f64") == 0) {
        return lexNextVarType(lexer, TOKEN_F64, buffer, TOKEN_FLOAT_VALUE);
    } else if (strcmp(buffer, "str") == 0) {
        return lexNextVarType(lexer, TOKEN_STRING, buffer, TOKEN_STRING_VALUE);
    } else if (strcmp(buffer, "bool") == 0) {
        return lexNextVarType(lexer, TOKEN_BOOLEAN, buffer, TOKEN_BOOLEAN_VALUE);
    } else if (strcmp(buffer, "true") == 0) {
        return tokenInit(TOKEN_BOOLEAN_VALUE, buffer);
    } else if (strcmp(buffer, "false") == 0) {
        return tokenInit(TOKEN_BOOLEAN_VALUE, buffer);
    } else if (strcmp(buffer, "type") == 0) {
        return tokenInit(TOKEN_TYPE_DEF, buffer);
    } else if (strcmp(buffer, "enum") == 0) {
        return tokenInit(TOKEN_ENUM_DEF, buffer);
    } else if (strcmp(buffer, "if") == 0) {
        return tokenInit(TOKEN_IF, buffer);
    } else if (strcmp(buffer, "else") == 0) {
        return tokenInit(TOKEN_ELSE, buffer);
    } else if (strcmp(buffer, "while") == 0) {
        return tokenInit(TOKEN_WHILE, buffer);
    } else if (strcmp(buffer, "for") == 0) {
        return tokenInit(TOKEN_FOR, buffer);
    } else if (strcmp(buffer, "return") == 0) {
        return tokenInit(TOKEN_RETURN, buffer);
    } else if (strcmp(buffer, "import") == 0) {
        return lexNextImport(lexer);
    }

    //Assume if it's not a reserved identifier, it must be a name
    return tokenInit(TOKEN_IDENTIFIER, buffer);
}

Token* lexNextString(Lexer* lexer) {
    char* buffer = charToStr(lexer->cur); //Contains first string char
    advance(lexer); //Move to next string char
    while (lexer->cur != '"') { //Grow with chars until closing string char
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strncat(buffer, lexer->contents + lexer->pos, 1);
        advance(lexer);
    }
    advance(lexer); //Advance past closing string char
    return tokenInit(TOKEN_STRING_VALUE, buffer);
}

Token* lexNextComment(Lexer* lexer) {
    char end = lexer->cur == '/' ? '\n' : '*'; //Keep track if this comment should end with \n or *
    advance(lexer); //Advance past that starting character and being reading the comment text
    char* buffer = charToStr(lexer->cur); //Buffer initialized with first comment char
    advance(lexer); //Advance past first comment char, since it is now in the initial buffer
    int type = TOKEN_COMMENT_LINE;
    //TODO remove need for len bounds check by refactoring lexer to lex line by line
    while (lexer->cur != end /*|| lexer->pos <= lexer->len*/) {
        buffer = realloc(buffer, (strlen(buffer) + 2) * sizeof(char));
        strncat(buffer, lexer->contents + lexer->pos, 1);
        advance(lexer);
    }
    if (end == '*') { //Advance twice to move past the trailing */ of multi line comments
        advance(lexer);
        advance(lexer);
        type = TOKEN_COMMENT_MULTI;
    }
    return tokenInit(type, buffer);
}

Token* lexNextToken(Lexer* lexer) {
    skipWhitespace(lexer);

    if (lexer->pos >= lexer->len) {
        return tokenInit(TOKEN_EOF, "EOF");
    } else if (lexer->cur == 'c' && peek(lexer, 1) == '.') {
        advance(lexer);
        advance(lexer); //Advance twice to move past "c." prefix
        return lexNextCStatement(lexer);
    } else if (isdigit(lexer->cur)) {
        return lexNextDigit(lexer);
    } else if (isalnum(lexer->cur)) {
        return lexNextIdentifier(lexer);
    }

    char* value = charToStr(lexer->cur);

    if (lexer->cur == ';') {
        advance(lexer);
        return tokenInit(TOKEN_EOS, value);
    } else if (lexer->cur == '=') {
        advance(lexer);
        if (lexer->cur == '=') {
            free(value);
            advance(lexer);
            return tokenInit(TOKEN_EQUALS, "==");
        } else {
            return tokenInit(TOKEN_ASSIGNMENT, value);
        }
    } else if (lexer->cur == '"') {
        advance(lexer);
        return lexNextString(lexer);
    } else if (lexer->cur == '/') {
        advance(lexer);
        if (lexer->cur == '/' || lexer->cur == '*') {
            return lexNextComment(lexer);
        }
        return tokenInit(TOKEN_DIVISION, value);
    } else if (lexer->cur == '+') {
        advance(lexer);
        return tokenInit(TOKEN_PLUS, value);
    } else if (lexer->cur == '-') {
        advance(lexer);
        return tokenInit(TOKEN_MINUS, value);
    } else if (lexer->cur == '*') {
        advance(lexer);
        return tokenInit(TOKEN_MULTIPLY, value);
    } else if (lexer->cur == '<') {
        advance(lexer);
        return tokenInit(TOKEN_LESS_THAN, value);
    } else if (lexer->cur == '>') {
        advance(lexer);
        return tokenInit(TOKEN_MORE_THAN, value);
    } else if (lexer->cur == '!') {
        advance(lexer);
        return tokenInit(TOKEN_NOT, value);
    } else if (lexer->cur == '(') {
        advance(lexer);
        return tokenInit(TOKEN_LPAREN, value);
    } else if (lexer->cur == ')') {
        advance(lexer);
        return tokenInit(TOKEN_RPAREN, value);
    } else if (lexer->cur == ':') {
        advance(lexer);
        return tokenInit(TOKEN_COLON, value);
    } else if (lexer->cur == ',') {
        advance(lexer);
        return tokenInit(TOKEN_COMMA, value);
    } else if (lexer->cur == '.') {
        advance(lexer);
        return tokenInit(TOKEN_DOT, value);
    } else if (lexer->cur == '[') {
        advance(lexer);
        return tokenInit(TOKEN_LBRACKET, value);
    } else if (lexer->cur == ']') {
        advance(lexer);
        return tokenInit(TOKEN_RBRACKET, value);
    } else if (lexer->cur == '{') {
        advance(lexer);
        return tokenInit(TOKEN_LBRACE, value);
    } else if (lexer->cur == '}') {
        advance(lexer);
        return tokenInit(TOKEN_RBRACE, value);
    }

    //Finally, return unexpected token is nothing matches
    advance(lexer);
    return tokenInit(TOKEN_UNEXPECTED, value);
}
#endif