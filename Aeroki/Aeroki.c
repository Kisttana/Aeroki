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

// ==== GUI Input Callback System ====
char *(*__Aeroki_GUI_Input_Callback)(const char *prompt) = NULL;

// ---- GUI-safe get_input_value() ----
int get_input_value(const char *prompt) {
    if (__Aeroki_GUI_Input_Callback) {
        char *val = __Aeroki_GUI_Input_Callback(prompt);
        if (val) {
            int num = atoi(val);
            free(val);
            return num;
        }
    }

    // GUI-safe fallback: send special message to GUI
    printf("__AEROKI_INPUT_REQUEST__%s\n", prompt);
    fflush(stdout);

    char buf[64];
    if (fgets(buf, sizeof(buf), stdin)) {
        int val = atoi(buf);
        return val;
    }
    return 0;
}

// ==== Lexer ====

typedef enum {
    TOK_GIVE, TOK_FIND, TOK_INPUT,
    TOK_IF, TOK_ELSE,
    TOK_ID, TOK_NUM,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_LT, TOK_GT, TOK_LE, TOK_GE, TOK_EQEQ, TOK_NEQ,
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
        if (isspace((unsigned char)*p)) { p++; continue; }

        if (strncmp(p, "ให้", strlen("ให้")) == 0) {
            tokens[tok_count++] = (Token){TOK_GIVE, "ให้"}; p += strlen("ให้");
        } else if (strncmp(p, "หา", strlen("หา")) == 0) {
            tokens[tok_count++] = (Token){TOK_FIND, "หา"}; p += strlen("หา");
        } else if (strncmp(p, "รับค่า", strlen("รับค่า")) == 0) {
            tokens[tok_count++] = (Token){TOK_INPUT, "รับค่า"}; p += strlen("รับค่า");
        } else if (strncmp(p, "ถ้าไม่", strlen("ถ้าไม่")) == 0) {
            tokens[tok_count++] = (Token){TOK_ELSE, "ถ้าไม่"}; p += strlen("ถ้าไม่");
        } else if (strncmp(p, "ถ้า", strlen("ถ้า")) == 0) {
            tokens[tok_count++] = (Token){TOK_IF, "ถ้า"}; p += strlen("ถ้า");
        }
        else if (*p == '<') {
            if (*(p+1) == '=') { tokens[tok_count++] = (Token){TOK_LE, "<="}; p+=2; }
            else { tokens[tok_count++] = (Token){TOK_LT, "<"}; p++; }
        } else if (*p == '>') {
            if (*(p+1) == '=') { tokens[tok_count++] = (Token){TOK_GE, ">="}; p+=2; }
            else { tokens[tok_count++] = (Token){TOK_GT, ">"}; p++; }
        } else if (*p == '=') {
            if (*(p+1) == '=') { tokens[tok_count++] = (Token){TOK_EQEQ, "=="}; p+=2; }
            else { tokens[tok_count++] = (Token){TOK_ASSIGN, "="}; p++; }
        } else if (*p == '!') {
            if (*(p+1) == '=') { tokens[tok_count++] = (Token){TOK_NEQ, "!="}; p+=2; }
            else p++;
        } else if (*p == '+') {
            tokens[tok_count++] = (Token){TOK_PLUS, "+"}; p++;
        } else if (*p == '-') {
            tokens[tok_count++] = (Token){TOK_MINUS, "-"}; p++;
        } else if (*p == '*') {
            tokens[tok_count++] = (Token){TOK_MUL, "*"}; p++;
        } else if (*p == '/') {
            tokens[tok_count++] = (Token){TOK_DIV, "/"}; p++;
        } else if (isdigit((unsigned char)*p)) {
            char buf[64]; int len = 0;
            while (isdigit((unsigned char)*p)) buf[len++] = *p++;
            buf[len] = '\0';
            tokens[tok_count++] = (Token){TOK_NUM, ""};
            strcpy(tokens[tok_count-1].text, buf);
        } else {
            char buf[64]; int len = 0;
            while (*p && !isspace((unsigned char)*p) && *p!='=' && *p!='+' && *p!='-' && *p!='*' && *p!='/'
                   && *p!='<' && *p!='>' && *p!='!') {
                buf[len++] = *p++;
            }
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

// ==== Parser ====

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

// ==== Function System ====

typedef struct {
    char name[64];
    char *lines[MAX_CMDS];
    int line_count;
} Function;

Function functions[MAX_VARIABLES];
int func_count = 0;
int in_function_def = 0;
char current_func_name[64];

void add_function_line(const char *line) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, current_func_name) == 0) {
            if (functions[i].line_count < MAX_CMDS) {
                functions[i].lines[functions[i].line_count] = strdup(line);
                functions[i].line_count++;
            }
            return;
        }
    }
}

void define_function_start(const char *name) {
    if (func_count < MAX_VARIABLES) {
        strcpy(functions[func_count].name, name);
        functions[func_count].line_count = 0;
        func_count++;
        strcpy(current_func_name, name);
        in_function_def = 1;
    } else {
        fprintf(stderr, "Too many functions.\n");
        exit(1);
    }
}

void define_function_end() {
    in_function_def = 0;
    current_func_name[0] = '\0';
}

Function *find_function(const char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0)
            return &functions[i];
    }
    return NULL;
}

void call_function(const char *name) {
    Function *f = find_function(name);
    if (!f) {
        fprintf(stderr, "ไม่พบฟังก์ชัน: %s\n", name);
        return;
    }
    for (int i = 0; i < f->line_count; i++) {
        add_command(f->lines[i]);
        run_all_commands();
    }
}

// ==== Condition Eval ====

int eval_condition_from_tokpos() {
    Node *left_expr = parse_expr();
    int left_val = eval(left_expr);

    Token *cmp = peek();
    if (cmp->type == TOK_LT || cmp->type == TOK_GT || cmp->type == TOK_LE || cmp->type == TOK_GE
        || cmp->type == TOK_EQEQ || cmp->type == TOK_NEQ) {
        TokenType cmpType = cmp->type;
        next();
        Node *right_expr = parse_expr();
        int right_val = eval(right_expr);

        switch (cmpType) {
            case TOK_LT: return left_val < right_val;
            case TOK_GT: return left_val > right_val;
            case TOK_LE: return left_val <= right_val;
            case TOK_GE: return left_val >= right_val;
            case TOK_EQEQ: return left_val == right_val;
            case TOK_NEQ: return left_val != right_val;
            default: return 0;
        }
    } else {
        return left_val != 0;
    }
}

// ==== Command Runner ====

void run_all_commands() {
    for (int i = 0; i < cmd_count; i++) {
        lex_line(cmd_buffer[i]);

        if (tokens[0].type == TOK_IF) {
            tok_pos = 1;
            int cond = eval_condition_from_tokpos();
            if (cond) {
                if (i + 1 < cmd_count) {
                    lex_line(cmd_buffer[i + 1]);
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
                            int val = get_input_value(tokens[1].text);
                            set_variable(tokens[1].text, val);
                        }
                    }
                    if (i + 1 < cmd_count) {
                        lex_line(cmd_buffer[i + 1]);
                        if (tokens[0].type == TOK_ELSE) {
                            i += 2;
                        } else {
                            i += 1;
                        }
                    } else {
                        i += 1;
                    }
                }
            } else {
                if (i + 1 < cmd_count) {
                    lex_line(cmd_buffer[i + 1]);
                    if (tokens[0].type == TOK_ELSE) {
                        if (i + 2 < cmd_count) {
                            lex_line(cmd_buffer[i + 2]);
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
                                    int val = get_input_value(tokens[1].text);
                                    set_variable(tokens[1].text, val);
                                }
                            }
                            i += 2;
                        } else {
                            i += 1;
                        }
                    }
                }
            }
        }
        else if (tokens[0].type == TOK_GIVE) {
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
                int val = get_input_value(tokens[1].text);
                set_variable(tokens[1].text, val);
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

        if (strncmp(line, "สร้าง ", strlen("สร้าง ")) == 0) {
            const char *name = line + strlen("สร้าง ");
            define_function_start(name);
            printf("เริ่มสร้างฟังก์ชัน: %s\n", name);
            continue;
        }
        if (strcmp(line, "จบฟังก์ชัน") == 0) {
            define_function_end();
            printf("สิ้นสุดฟังก์ชัน\n");
            continue;
        }
        if (strncmp(line, "เรียก ", strlen("เรียก ")) == 0) {
            const char *name = line + strlen("เรียก ");
            call_function(name);
            continue;
        }

        if (in_function_def) {
            add_function_line(line);
        } else {
            add_command(line);
            run_all_commands();
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

        if (strncmp(line, "สร้าง ", strlen("สร้าง ")) == 0) {
            const char *name = line + strlen("สร้าง ");
            define_function_start(name);
            continue;
        }
        if (strcmp(line, "จบฟังก์ชัน") == 0) {
            define_function_end();
            continue;
        }
        if (strncmp(line, "เรียก ", strlen("เรียก ")) == 0) {
            const char *name = line + strlen("เรียก ");
            call_function(name);
            continue;
        }

        if (in_function_def) {
            add_function_line(line);
        } else {
            add_command(line);
            run_all_commands();
        }
    }
}
