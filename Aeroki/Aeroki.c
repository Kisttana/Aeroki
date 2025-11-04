#include "Aeroki.h"

#define MAX_VARIABLES 100
#define MAX_CMDS 256

typedef struct {
    char name[32];
    int value;
} Variable;

Variable variables[MAX_VARIABLES];
int var_count = 0;

int get_variable(const char *name) {
    for (int i = 0; i < var_count; ++i)
        if (strcmp(variables[i].name, name) == 0)
            return variables[i].value;
    fprintf(stderr, "Variable not found: %s\n", name);
    exit(1);
}

void set_variable(const char *name, int value) {
    for (int i = 0; i < var_count; ++i)
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    if (var_count < MAX_VARIABLES) {
        strcpy(variables[var_count].name, name);
        variables[var_count].value = value;
        var_count++;
    } else {
        fprintf(stderr, "Too many variables.\n");
        exit(1);
    }
}

// ===== LEXER =====

typedef enum {
    TOK_GIVE, TOK_FIND, TOK_INPUT, TOK_RESULT, TOK_PRINT,
    TOK_IF, TOK_ELSE,
    TOK_ID, TOK_NUM, TOK_STRING,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_LT, TOK_GT, TOK_EQEQ,
    TOK_LBRACE, TOK_RBRACE,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[128];
} Token;

Token tokens[512];
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
        } else if (strncmp(p, "แสดง", strlen("แสดง")) == 0) {
            tokens[tok_count++] = (Token){TOK_PRINT, "แสดง"}; p += strlen("แสดง");
        } else if (strncmp(p, "ถ้าไม่", strlen("ถ้าไม่")) == 0) {
            tokens[tok_count++] = (Token){TOK_ELSE, "ถ้าไม่"}; p += strlen("ถ้าไม่");
        } else if (strncmp(p, "ถ้า", strlen("ถ้า")) == 0) {
            tokens[tok_count++] = (Token){TOK_IF, "ถ้า"}; p += strlen("ถ้า");
        } else if (*p == '{') {
            tokens[tok_count++] = (Token){TOK_LBRACE, "{"}; p++;
        } else if (*p == '}') {
            tokens[tok_count++] = (Token){TOK_RBRACE, "}"}; p++;
        } else if (*p == '=') {
            if (*(p+1) == '=') { tokens[tok_count++] = (Token){TOK_EQEQ, "=="}; p += 2; }
            else { tokens[tok_count++] = (Token){TOK_ASSIGN, "="}; p++; }
        } else if (*p == '<') {
            tokens[tok_count++] = (Token){TOK_LT, "<"}; p++;
        } else if (*p == '>') {
            tokens[tok_count++] = (Token){TOK_GT, ">"}; p++;
        } else if (*p == '+') {
            tokens[tok_count++] = (Token){TOK_PLUS, "+"}; p++;
        } else if (*p == '-') {
            tokens[tok_count++] = (Token){TOK_MINUS, "-"}; p++;
        } else if (*p == '*') {
            tokens[tok_count++] = (Token){TOK_MUL, "*"}; p++;
        } else if (*p == '/') {
            tokens[tok_count++] = (Token){TOK_DIV, "/"}; p++;
        } else if (*p == '"') {
            p++;
            char buf[128]; int len = 0;
            while (*p && *p != '"') buf[len++] = *p++;
            buf[len] = '\0';
            if (*p == '"') p++;
            tokens[tok_count++] = (Token){TOK_STRING, ""};
            strcpy(tokens[tok_count-1].text, buf);
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

// ==== Expressions ====

int parse_expr();

int parse_factor() {
    Token *t = peek();
    if (t->type == TOK_NUM) {
        next(); return atoi(t->text);
    } else if (t->type == TOK_ID) {
        next(); return get_variable(t->text);
    }
    return 0;
}

int parse_term() {
    int val = parse_factor();
    while (peek()->type == TOK_MUL || peek()->type == TOK_DIV) {
        char op = next()->text[0];
        int right = parse_factor();
        if (op == '*') val *= right;
        else val /= right;
    }
    return val;
}

int parse_expr() {
    int val = parse_term();
    while (peek()->type == TOK_PLUS || peek()->type == TOK_MINUS) {
        char op = next()->text[0];
        int right = parse_term();
        if (op == '+') val += right;
        else val -= right;
    }
    return val;
}

// ==== Interpreter ====

void execute_block();

void execute_if() {
    next(); // skip ถ้า
    int condition_left = parse_expr();
    int truth = condition_left;

    if (peek()->type == TOK_LBRACE) next();
    if (truth) {
        execute_block();
        // skip else block if exists
        if (peek()->type == TOK_ELSE) {
            next();
            if (peek()->type == TOK_LBRACE) {
                int brace = 1;
                while (brace > 0) {
                    if (peek()->type == TOK_LBRACE) brace++;
                    else if (peek()->type == TOK_RBRACE) brace--;
                    next();
                }
            }
        }
    } else {
        // skip until else or end
        int brace = 1;
        while (brace > 0) {
            if (peek()->type == TOK_LBRACE) brace++;
            else if (peek()->type == TOK_RBRACE) brace--;
            next();
        }
        if (peek()->type == TOK_ELSE) {
            next();
            if (peek()->type == TOK_LBRACE) next();
            execute_block();
        }
    }
}

void execute_block() {
    while (peek()->type != TOK_RBRACE && peek()->type != TOK_EOF) {
        Token *t = peek();
        if (t->type == TOK_PRINT) {
            next();
            if (peek()->type == TOK_STRING) {
                printf("%s\n", peek()->text);
                next();
            }
        } else if (t->type == TOK_IF) {
            execute_if();
        } else {
            next();
        }
    }
    if (peek()->type == TOK_RBRACE) next();
}

// ==== Shell Interpreter ====

void __Ark_Shell() {
    char line[256];
    printf("Aeroki Shell Mode (type 'ออก' to exit)\n");
    while (1) {
        printf(">>> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        ltrim(line);
        if (strcmp(line, "ออก") == 0) break;
        if (line[0] == '\0') continue;

        lex_line(line);
        if (tokens[0].type == TOK_RESULT) {
            run_all_commands();
        } else {
            add_command(line);
        }
    }
}
