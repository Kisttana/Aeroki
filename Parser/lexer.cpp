#include "../Aeroki/Aeroki.h"

typedef enum {
    TOK_GIVE, TOK_FIND, TOK_INPUT, TOK_RESULT,
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

        if (strncmp(p, "ให้", strlen("ให้")) == 0) {
            tokens[tok_count++] = (Token){TOK_GIVE, "ให้"}; p += strlen("ให้");
        } else if (strncmp(p, "หา", strlen("หา")) == 0) {
            tokens[tok_count++] = (Token){TOK_FIND, "หา"}; p += strlen("หา");
        } else if (strncmp(p, "รับค่า", strlen("รับค่า")) == 0) {
            tokens[tok_count++] = (Token){TOK_INPUT, "รับค่า"}; p += strlen("รับค่า");
        } else if (strncmp(p, "ผล", strlen("ผล")) == 0) {
            tokens[tok_count++] = (Token){TOK_RESULT, "ผล"}; p += strlen("ผล");
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
