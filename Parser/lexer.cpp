#include "Aeroki.h"

Token tokens[128];
int tok_count = 0;
int tok_pos = 0;

void lex_line(const char *line) {
    tok_count = 0; tok_pos = 0;
    const char *p = line;

    while (*p) {
        if (isspace((unsigned char)*p)) { p++; continue; }

        if (strncmp(p, "ให้", strlen("ให้")) == 0) {
            tokens[tok_count++] = {TOK_GIVE, "ให้"}; p += strlen("ให้");
        } else if (strncmp(p, "หา", strlen("หา")) == 0) {
            tokens[tok_count++] = {TOK_FIND, "หา"}; p += strlen("หา");
        } else if (strncmp(p, "รับค่า", strlen("รับค่า")) == 0) {
            tokens[tok_count++] = {TOK_INPUT, "รับค่า"}; p += strlen("รับค่า");
        } else if (strncmp(p, "ผล", strlen("ผล")) == 0) {
            tokens[tok_count++] = {TOK_RESULT, "ผล"}; p += strlen("ผล");
        } else if (*p == '=') {
            tokens[tok_count++] = {TOK_ASSIGN, "="}; p++;
        } else if (*p == '+') {
            tokens[tok_count++] = {TOK_PLUS, "+"}; p++;
        } else if (*p == '-') {
            tokens[tok_count++] = {TOK_MINUS, "-"}; p++;
        } else if (*p == '*') {
            tokens[tok_count++] = {TOK_MUL, "*"}; p++;
        } else if (*p == '/') {
            tokens[tok_count++] = {TOK_DIV, "/"}; p++;
        } else if (isdigit((unsigned char)*p)) {
            char buf[64]; int len = 0;
            while (isdigit((unsigned char)*p)) buf[len++] = *p++;
            buf[len] = '\0';
            tokens[tok_count++] = {TOK_NUM, ""};
            strcpy(tokens[tok_count-1].text, buf);
        } else {
            char buf[64]; int len = 0;
            while (*p && !isspace((unsigned char)*p) && *p!='=' && *p!='+' && *p!='-' && *p!='*' && *p!='/')
                buf[len++] = *p++;
            buf[len] = '\0';
            tokens[tok_count++] = {TOK_ID, ""};
            strcpy(tokens[tok_count-1].text, buf);
        }
    }
    tokens[tok_count++] = {TOK_EOF, ""};
}

Token *peek() { return &tokens[tok_pos]; }
Token *next() { return &tokens[tok_pos++]; }
