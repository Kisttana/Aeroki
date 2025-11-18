#include "Aeroki.h"

#define MAX_VARIABLES 100
#define MAX_CMDS 256

// ==== Value (decimal-aware) ====

typedef struct {
    long double v;   // numeric value
    int scale;       // preferred decimals from source (e.g., 5.00 => 2, 5 => 0)
} Value;

static inline Value make_value(long double v, int scale) {
    Value x; x.v = v; x.scale = (scale < 0 ? 0 : scale); return x;
}

static inline int max_int(int a, int b) { return a > b ? a : b; }

static int g_out_dp = 2;    // default cap at 2
static int g_fixed_dp = 0;  // 0 = auto, 1 = fixed padding

// Round half away from zero to N decimals
static long double round_to(long double x, int dp) {
    if (dp < 0) dp = 0;
    long double p = powl(10.0L, (long double)dp);
    long double y = x * p;
    // Nudge y by a tiny epsilon proportional to its magnitude to avoid
    // binary-floating point being slightly below an exact halfway value
    // (e.g., 1355.4999999999999) which would round down incorrectly.
    long double eps = fabsl(y) * 1e-15L + 1e-18L;
    if (y >= 0) y += eps; else y -= eps;
    long double r = roundl(y); // roundl does round-half-away-from-zero per C standard
    return r / p;
}

static void print_value(Value val) {
    int decimals;
    if (g_fixed_dp) {
        decimals = g_out_dp;
    } else {
        if (val.scale > 0) {
            decimals = (val.scale < g_out_dp) ? val.scale : g_out_dp;
        } else {
            // If the stored scale is 0 but the value actually has a fractional
            // part (e.g. scale was lost), show up to g_out_dp decimals so
            // values like 3.14 aren't printed as "3".
            long double frac = fabsl(val.v - floorl(val.v));
            if (frac > 0.0L) decimals = g_out_dp;
            else decimals = 0;
        }
    }

    long double rv = round_to(val.v, decimals);
    // Use a generated format string but print as `double` with `%%f`.
    // Many Windows/MSVC runtimes do not support `%%Lf`, so casting to
    // `double` and using `%%f` is the most portable choice here.
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%.%df\n", decimals);
    // Debug: print internal info to stderr when AEROKI_DEBUG is set.
    if (getenv("AEROKI_DEBUG")) {
        fprintf(stderr, "[debug] val.v=%.*Lg val.scale=%d decimals=%d rv=%.*Lg\n",
                12, val.v, val.scale, decimals, 12, rv);
    }

    printf(fmt, (double)rv);
}

// Parse decimal lexeme "123", "5.0", "5.00", "5.005"
static Value parse_decimal_lexeme(const char *lex) {
    int scale = 0;
    const char *dot = strchr(lex, '.');
    if (dot) {
        scale = (int)strlen(dot + 1);
    }
    // Note: keep exact scale from lexeme (including trailing zeros)
    long double v = strtold(lex, NULL);
    return make_value(v, scale);
}

// ==== Variable ====

typedef struct {
    char name[32];
    Value value;
} Variable;

Variable variables[MAX_VARIABLES];
int var_count = 0;

Value get_variable(const char *name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Variable not found: %s\n", name);
    exit(1);
}

void set_variable(const char *name, Value value) {
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
    TOK_GIVE, TOK_FIND, TOK_INPUT,
    TOK_IF, TOK_ELSE,
    TOK_PREC, // ทศนิยม
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
        } else if (strncmp(p, "ทศนิยม", strlen("ทศนิยม")) == 0) {
            tokens[tok_count++] = (Token){TOK_PREC, "ทศนิยม"}; p += strlen("ทศนิยม");
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
            // number with optional decimal part
            char buf[64]; int len = 0;
            while (isdigit((unsigned char)*p)) buf[len++] = *p++;
            if (*p == '.') {
                buf[len++] = *p++; // dot
                while (isdigit((unsigned char)*p)) buf[len++] = *p++;
            }
            buf[len] = '\0';
            tokens[tok_count++] = (Token){TOK_NUM, ""};
            strcpy(tokens[tok_count-1].text, buf);
        } else {
            char buf[64]; int len = 0;
            while (*p && !isspace((unsigned char)*p) && *p!='=' && *p!='+' && *p!='-' && *p!='*' && *p!='/'
                   && *p!='<' && *p!='>' && *p!='!' && *p!='.') {
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
    long double fvalue;
    int scale;
    char varname[32];
    char op;
    struct Node *left, *right;
} Node;

Node *make_num_ld(long double v, int scale) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_NUM; n->fvalue = v; n->scale = scale;
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
        Value val = parse_decimal_lexeme(t->text);
        return make_num_ld(val.v, val.scale);
    } else if (t->type == TOK_ID) {
        next();
        return make_var(t->text);
    }
    return make_num_ld(0.0L, 0);
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

static Value eval_node(struct Node *n);

static Value bin_calc(char op, Value l, Value r) {
    Value out;
    switch (op) {
        case '+': out.v = l.v + r.v; out.scale = max_int(l.scale, r.scale); break;
        case '-': out.v = l.v - r.v; out.scale = max_int(l.scale, r.scale); break;
        case '*': out.v = l.v * r.v; out.scale = max_int(l.scale, r.scale); break;
        case '/':
            if (r.v == 0.0L) { out.v = 0.0L; out.scale = 0; }
            else { out.v = l.v / r.v; out.scale = max_int(l.scale, r.scale); }
            break;
        default: out.v = 0.0L; out.scale = 0; break;
    }
    return out;
}

static Value eval_node(struct Node *n) {
    if (n->type == NODE_NUM) return make_value(n->fvalue, n->scale);
    if (n->type == NODE_VAR) return get_variable(n->varname);
    if (n->type == NODE_BINOP) {
        Value l = eval_node(n->left), r = eval_node(n->right);
        return bin_calc(n->op, l, r);
    }
    return make_value(0.0L, 0);
}

// ==== Command Buffer ====

char *cmd_buffer[MAX_CMDS];
int cmd_count = 0;

void add_command(const char *line) {
    if (cmd_count < MAX_CMDS) {
        cmd_buffer[cmd_count] = _strdup(line);
        cmd_count++;
    }
}

// helper: evaluate condition with comparison operators
int eval_condition_from_tokpos() {
    Node *left_expr = parse_expr();
    Value left_val = eval_node(left_expr);

    Token *cmp = peek();
    if (cmp->type == TOK_LT || cmp->type == TOK_GT || cmp->type == TOK_LE || cmp->type == TOK_GE
        || cmp->type == TOK_EQEQ || cmp->type == TOK_NEQ) {
        TokenType cmpType = cmp->type;
        next(); // consume comparator
        Node *right_expr = parse_expr();
        Value right_val = eval_node(right_expr);

        long double L = left_val.v, R = right_val.v;
        switch (cmpType) {
            case TOK_LT: return L < R;
            case TOK_GT: return L > R;
            case TOK_LE: return L <= R;
            case TOK_GE: return L >= R;
            case TOK_EQEQ: return L == R;
            case TOK_NEQ: return L != R;
            default: return 0;
        }
    } else {
        return left_val.v != 0.0L;
    }
}

// parse user input string to Value (preserve trailing zeros)
static Value parse_input_value() {
    char buf[256];
    if (!fgets(buf, sizeof(buf), stdin)) {
        return make_value(0.0L, 0);
    }
    buf[strcspn(buf, "\r\n")] = '\0';
    // trim spaces
    char *s = buf;
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return make_value(0.0L, 0);

    // detect scale
    int scale = 0;
    char *dot = strchr(s, '.');
    if (dot) scale = (int)strlen(dot + 1);

    long double v = strtold(s, NULL);
    return make_value(v, scale);
}

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
                            Value val = eval_node(expr);
                            set_variable(tokens[1].text, val);
                        }
                    } else if (tokens[0].type == TOK_FIND) {
                        tok_pos = 1;
                        Node *expr = parse_expr();
                        print_value(eval_node(expr));
                    } else if (tokens[0].type == TOK_INPUT) {
                        if (tokens[1].type == TOK_ID) {
                            printf("กรอกค่า %s: ", tokens[1].text);
                            fflush(stdout);
                            Value v = parse_input_value();
                            set_variable(tokens[1].text, v);
                        }
                    } else if (tokens[0].type == TOK_PREC) {
                        if (tokens[1].type == TOK_NUM) {
                            g_out_dp = (int)strtol(tokens[1].text, NULL, 10);
                            if (g_out_dp < 0) g_out_dp = 0;
                            if (g_out_dp > 12) g_out_dp = 12; // reasonable cap
                            g_fixed_dp = 1;
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
                                    Value val = eval_node(expr);
                                    set_variable(tokens[1].text, val);
                                }
                            } else if (tokens[0].type == TOK_FIND) {
                                tok_pos = 1;
                                Node *expr = parse_expr();
                                print_value(eval_node(expr));
                            } else if (tokens[0].type == TOK_INPUT) {
                                if (tokens[1].type == TOK_ID) {
                                    printf("กรอกค่า %s: ", tokens[1].text);
                                    fflush(stdout);
                                    Value v = parse_input_value();
                                    set_variable(tokens[1].text, v);
                                }
                            } else if (tokens[0].type == TOK_PREC) {
                                if (tokens[1].type == TOK_NUM) {
                                    g_out_dp = (int)strtol(tokens[1].text, NULL, 10);
                                    if (g_out_dp < 0) g_out_dp = 0;
                                    if (g_out_dp > 12) g_out_dp = 12;
                                    g_fixed_dp = 1;
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
                Value val = eval_node(expr);
                set_variable(tokens[1].text, val);
            }
        } else if (tokens[0].type == TOK_FIND) {
            tok_pos = 1;
            Node *expr = parse_expr();
            print_value(eval_node(expr));
        } else if (tokens[0].type == TOK_INPUT) {
            if (tokens[1].type == TOK_ID) {
                printf("กรอกค่า %s: ", tokens[1].text);
                fflush(stdout);
                Value v = parse_input_value();
                set_variable(tokens[1].text, v);
            }
        } else if (tokens[0].type == TOK_PREC) {
            if (tokens[1].type == TOK_NUM) {
                g_out_dp = (int)strtol(tokens[1].text, NULL, 10);
                if (g_out_dp < 0) g_out_dp = 0;
                if (g_out_dp > 12) g_out_dp = 12;
                g_fixed_dp = 1;
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
    // Ensure numeric parsing/printing uses C locale (decimal '.'),
    // useful when running inside GUIs which may inherit different locales.
    setlocale(LC_NUMERIC, "C");
    printf("Aeroki Shell Mode (type 'ออก' to exit)\n");
    while (1) {
        printf(">>> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        ltrim(line);
        if (strcmp(line, "ออก") == 0) break;
        if (line[0] == '\0') continue;

        add_command(line);
        run_all_commands();
    }
}

// ==== File Interpreter ====

void __Ark_Interpreted(FILE *src) {
    char line[256];
    // Ensure numeric parsing/printing uses C locale (decimal '.') for file mode.
    setlocale(LC_NUMERIC, "C");

    while (fgets(line, sizeof(line), src)) {
        line[strcspn(line, "\n")] = '\0';
        ltrim(line);
        if (line[0] == '\0') continue;

        add_command(line);
        run_all_commands();
    }
}
