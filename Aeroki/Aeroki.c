#include "Aeroki.h"

#define MAX_VARIABLES 100
#define MAX_CMDS 256

// ==== Variable ====

typedef struct {
    char name[32];
    int value;
} Variable;

Variable variables[MAX_VARIABLES];
int var_count = 0;

int get_variable(const char *name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Variable not found: %s\n", name);
    exit(1);
}

void set_variable(const char *name, int value) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
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

// ==== Lexer ====

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

// ==== AST ====

typedef enum { NODE_NUM, NODE_VAR, NODE_BINOP } NodeType;

typedef struct Node {
    NodeType type;
    int value;
    char varname[32];
    char op;
    struct Node *left, *right;
} Node;

Node *make_num(int v) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_NUM; n->value = v;
    n->left = n->right = NULL;
    return n;
}
Node *make_var(const char *name) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_VAR; strcpy(n->varname, name);
    n->left = n->right = NULL;
    return n;
}
Node *make_binop(char op, Node *l, Node *r) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_BINOP; n->op = op;
    n->left = l; n->right = r;
    return n;
}

// ==== Parser (recursive descent) ====

Node *parse_expr();

Node *parse_factor() {
    Token *t = peek();
    if (t->type == TOK_NUM) {
        next();
        return make_num(atoi(t->text));
    } else if (t->type == TOK_ID) {
        next();
        return make_var(t->text);
    }
    return make_num(0);
}

Node *parse_term() {
    Node *node = parse_factor();
    while (peek()->type == TOK_MUL || peek()->type == TOK_DIV) {
        char op = peek()->text[0];
        next();
        node = make_binop(op, node, parse_factor());
    }
    return node;
}
Node *parse_expr() {
    Node *node = parse_term();
    while (peek()->type == TOK_PLUS || peek()->type == TOK_MINUS) {
        char op = peek()->text[0];
        next();
        node = make_binop(op, node, parse_term());
    }
    return node;
}

// ==== Interpreter ====

int eval(Node *n) {
    if (n->type == NODE_NUM) return n->value;
    if (n->type == NODE_VAR) return get_variable(n->varname);
    if (n->type == NODE_BINOP) {
        int l = eval(n->left), r = eval(n->right);
        switch (n->op) {
            case '+': return l + r;
            case '-': return l - r;
            case '*': return l * r;
            case '/': return r ? l / r : 0;
        }
    }
    return 0;
}

// ==== Command Buffer ====

char *cmd_buffer[MAX_CMDS];
int cmd_count = 0;

void add_command(const char *line) {
    if (cmd_count < MAX_CMDS) {
        cmd_buffer[cmd_count] = strdup(line);
        cmd_count++;
    }
}

void run_all_commands() {
    for (int i = 0; i < cmd_count; i++) {
        lex_line(cmd_buffer[i]);
        // run the command
        if (tokens[0].type == TOK_GIVE) {
            if (tokens[1].type == TOK_ID && tokens[2].type == TOK_ASSIGN) {
                tok_pos = 3;
                Node *expr = parse_expr();
                int val = eval(expr);
                set_variable(tokens[1].text, val);
            }
        } else if (tokens[0].type == TOK_FIND) {
            tok_pos = 1;
            Node *expr = parse_expr();
            printf("%d\n", eval(expr));
        } else if (tokens[0].type == TOK_INPUT) {
            if (tokens[1].type == TOK_ID) {
                int val;
                printf("กรอกค่า %s: ", tokens[1].text);
                fflush(stdout);
                if (scanf("%d", &val) == 1) {
                    set_variable(tokens[1].text, val);
                } else {
                    printf("Invalid input.\n");
                    int c; while ((c = getchar()) != '\n' && c != EOF);
                }
            }
        }
        free(cmd_buffer[i]);
    }
    cmd_count = 0;
}

// ==== Helpers ====

static void ltrim(char *str) {
    int index = 0;
    while (str[index] == ' ' || str[index] == '\t') index++;
    if (index > 0) {
        int i = 0;
        while (str[index]) str[i++] = str[index++];
        str[i] = '\0';
    }
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

// ==== File Interpreter ====

void __Ark_Interpreted(FILE *src) {
    char line[256];

    while (fgets(line, sizeof(line), src)) {
        line[strcspn(line, "\n")] = '\0';
        ltrim(line);
        if (line[0] == '\0') continue;

        lex_line(line);
        if (tokens[0].type == TOK_RESULT) {
            run_all_commands();
        } else {
            add_command(line);
        }
    }
}
