#include "Aeroki.h"

#define MAX_VARIABLES 100

// ==== Variable ====
typedef struct {
    char name[32];
    int value;
} Variable;

Variable variables[MAX_VARIABLES];
int var_count = 0;

int get_variable(const char *name) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0)
            return variables[i].value;
    }
    fprintf(stderr, "Variable not found: %s\n", name);
    return 0;
}

void set_variable(const char *name, int value) {
    for (int i = 0; i < var_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    strcpy(variables[var_count].name, name);
    variables[var_count].value = value;
    var_count++;
}

// ==== Lexer ====
typedef enum {
    TOK_GIVE, TOK_FIND, TOK_INPUT,
    TOK_ID, TOK_NUM,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[64];
} Token;

Token tokens[128];
int tok_count, tok_pos;

void lex_line(const char *line) {
    tok_count = 0; tok_pos = 0;
    const char *p = line;

    while (*p) {
        if (isspace(*p)) { p++; continue; }

        if (strncmp(p, "ให้", 6) == 0) {
            tokens[tok_count++] = (Token){TOK_GIVE, "ให้"}; p += 6;
        } else if (strncmp(p, "หา", 6) == 0) {
            tokens[tok_count++] = (Token){TOK_FIND, "หา"}; p += 6;
        } else if (*p == '=') {
            tokens[tok_count++] = (Token){TOK_ASSIGN, "="}; p++;
        } else if (*p == '+') {
            tokens[tok_count++] = (Token){TOK_PLUS, "+"}; p++;
        } else if (*p == '-') {
            tokens[tok_count++] = (Token){TOK_MINUS, "-"}; p++;
        } else if (*p == '*') {
            tokens[tok_count++] = (Token){TOK_MUL, "*"}; p++;
        } else if (*p == '/') {
            tokens[tok_count++] = (Token){TOK_DIV, "/"}; p++;
        } else if (isdigit(*p)) {
            char buf[64]; int len = 0;
            while (isdigit(*p)) buf[len++] = *p++;
            buf[len] = '\0';
            tokens[tok_count++] = (Token){TOK_NUM, ""};
            strcpy(tokens[tok_count-1].text, buf);
        } else {
            char buf[64]; int len = 0;
            while (*p && !isspace(*p) && *p!='=' && *p!='+' && *p!='-' && *p!='*' && *p!='/')
                buf[len++] = *p++;
            buf[len] = '\0';
            tokens[tok_count++] = (Token){TOK_ID, ""};
            strcpy(tokens[tok_count-1].text, buf);
        }
    }
    tokens[tok_count++] = (Token){TOK_EOF, ""};
}

Token *peek() { return &tokens[tok_pos]; }
Token *next() { return &tokens[tok_pos++]; }
