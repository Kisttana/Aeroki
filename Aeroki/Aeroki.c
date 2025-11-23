#include "Aeroki.h"

#define MAX_VARIABLES 100
#define MAX_CMDS 256
#define MAX_ARRAYS 50
#define MAX_ARRAY_SIZE 1000
#define MAX_FUNCTIONS 50
#define MAX_FUNC_PARAMS 10

// ==== Value (decimal-aware) ====

typedef struct {
    long double v;
    int scale;
} Value;

static inline Value make_value(long double v, int scale) {
    Value x; x.v = v; x.scale = (scale < 0 ? 0 : scale); return x;
}

static inline int max_int(int a, int b) { return a > b ? a : b; }

static int g_out_dp = 2;
static int g_fixed_dp = 0;

// Round half away from zero to N decimals
static long double round_to(long double x, int dp) {
    if (dp < 0) dp = 0;
    long double p = powl(10.0L, (long double)dp);
    long double y = x * p;
    long double eps = fabsl(y) * 1e-15L + 1e-18L;
    if (y >= 0) y += eps; else y -= eps;
    long double r = roundl(y);
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
            decimals = 0;
        }
    }

    long double rv = round_to(val.v, decimals);
    char fmt[16];
    snprintf(fmt, sizeof(fmt), "%%.%df\n", decimals);
    if (getenv("AEROKI_DEBUG")) {
        fprintf(stderr, "[debug] val.v=%.*Lg val.scale=%d decimals=%d rv=%.*Lg\n",
                15, val.v, val.scale, decimals, 15, rv);
    }
    printf(fmt, (double)rv);
}

static Value parse_decimal_lexeme(const char *lex) {
    int scale = 0;
    const char *dot = strchr(lex, '.');
    if (dot) {
        scale = (int)strlen(dot + 1);
    }
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

int has_variable(const char *name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            return 1;
        }
    }
    return 0;
}

Value get_variable(const char *name) {
    for (int i = 0; i < var_count; ++i) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    // Return default value instead of exiting immediately
    // This allows variables to be created on first use
    return make_value(0.0L, 0);
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

// ==== Arrays ====

typedef struct {
    char name[32];
    Value data[MAX_ARRAY_SIZE];
    int size;
    int capacity;
} Array;

Array arrays[MAX_ARRAYS];
int array_count = 0;

Array* get_array(const char *name) {
    for (int i = 0; i < array_count; i++) {
        if (strcmp(arrays[i].name, name) == 0) {
            return &arrays[i];
        }
    }
    return NULL;
}

void create_array(const char *name, int size) {
    if (array_count >= MAX_ARRAYS) {
        fprintf(stderr, "Too many arrays.\n");
        exit(1);
    }
    strcpy(arrays[array_count].name, name);
    arrays[array_count].size = 0;  // Start with 0 elements, not size!
    arrays[array_count].capacity = size > 0 ? size : MAX_ARRAY_SIZE;
    for (int i = 0; i < arrays[array_count].capacity; i++) {
        arrays[array_count].data[i] = make_value(0.0L, 0);
    }
    array_count++;
}

void array_push(const char *name, Value val) {
    Array *arr = get_array(name);
    if (!arr) {
        fprintf(stderr, "Array not found: %s\n", name);
        exit(1);
    }
    if (arr->size >= arr->capacity) {
        fprintf(stderr, "Array is full: %s\n", name);
        exit(1);
    }
    arr->data[arr->size++] = val;
}

Value array_pop(const char *name) {
    Array *arr = get_array(name);
    if (!arr) {
        fprintf(stderr, "Array not found: %s\n", name);
        exit(1);
    }
    if (arr->size == 0) {
        fprintf(stderr, "Array is empty: %s\n", name);
        exit(1);
    }
    return arr->data[--arr->size];
}

int array_length(const char *name) {
    Array *arr = get_array(name);
    if (!arr) {
        // Don't exit here, let caller handle it
        return -1;
    }
    return arr->size;
}

void array_set(const char *name, int index, Value val) {
    Array *arr = get_array(name);
    if (!arr) {
        fprintf(stderr, "Array not found: %s\n", name);
        exit(1);
    }
    
    // FIX #4: Auto-expand array if index exceeds size
    if (index >= arr->capacity) {
        fprintf(stderr, "Array index exceeds capacity: %d >= %d\n", index, arr->capacity);
        exit(1);
    }
    
    // Expand size if needed
    if (index >= arr->size) {
        // Fill gaps with zeros
        for (int i = arr->size; i < index; i++) {
            arr->data[i] = make_value(0.0L, 0);
        }
        arr->size = index + 1;
    }
    
    arr->data[index] = val;
}

// ==== Functions ====

typedef struct {
    char name[32];
    char params[MAX_FUNC_PARAMS][32];
    int param_count;
    char *body[MAX_CMDS];
    int body_count;
} Function;

Function functions[MAX_FUNCTIONS];
int func_count = 0;

Function* get_function(const char *name) {
    for (int i = 0; i < func_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            return &functions[i];
        }
    }
    return NULL;
}

void add_function(const char *name, char params[][32], int param_count, char **body, int body_count) {
    if (func_count >= MAX_FUNCTIONS) {
        fprintf(stderr, "Too many functions.\n");
        exit(1);
    }
    strcpy(functions[func_count].name, name);
    functions[func_count].param_count = param_count;
    for (int i = 0; i < param_count; i++) {
        strcpy(functions[func_count].params[i], params[i]);
    }
    functions[func_count].body_count = body_count;
    for (int i = 0; i < body_count; i++) {
        // Use strdup instead of _strdup for portability
        #ifdef _WIN32
            functions[func_count].body[i] = _strdup(body[i]);
        #else
            functions[func_count].body[i] = strdup(body[i]);
        #endif
    }
    func_count++;
}

// ==== Lexer ====

typedef enum {
    TOK_GIVE, TOK_FIND, TOK_INPUT,
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_BREAK, TOK_CONTINUE,
    TOK_PREC, TOK_FUNC, TOK_RETURN, TOK_CALL, TOK_END,
    TOK_ARRAY, TOK_PUSH, TOK_POP, TOK_LEN,
    TOK_AND, TOK_OR, TOK_NOT,
    TOK_MOD, TOK_POWER,
    TOK_SQRT, TOK_ABS, TOK_FLOOR, TOK_CEIL,
    TOK_PRINT, TOK_PRINTLN,
    TOK_ID, TOK_NUM, TOK_STRING,
    TOK_ASSIGN, TOK_PLUS, TOK_MINUS, TOK_MUL, TOK_DIV,
    TOK_PLUSEQ, TOK_MINUSEQ, TOK_MULEQ, TOK_DIVEQ,
    TOK_LT, TOK_GT, TOK_LE, TOK_GE, TOK_EQEQ, TOK_NEQ,
    TOK_LPAREN, TOK_RPAREN, TOK_LBRACKET, TOK_RBRACKET,
    TOK_COMMA,
    TOK_EOF
} TokenType;

typedef struct {
    TokenType type;
    char text[256];
} Token;

Token tokens[256];
int tok_count, tok_pos;

void lex_line(const char *line) {
    tok_count = 0; tok_pos = 0;
    const char *p = line;

    while (*p) {
        if (isspace((unsigned char)*p)) { p++; continue; }

        // FIX #2: Check longer keywords FIRST to prevent conflicts
        // Keywords (check longer ones first)
        if (strncmp(p, "แสดงบรรทัด", strlen("แสดงบรรทัด")) == 0) {
            tokens[tok_count++] = (Token){TOK_PRINTLN, "แสดงบรรทัด"}; p += strlen("แสดงบรรทัด");
        } else if (strncmp(p, "ค่าสัมบูรณ์", strlen("ค่าสัมบูรณ์")) == 0) {
            tokens[tok_count++] = (Token){TOK_ABS, "ค่าสัมบูรณ์"}; p += strlen("ค่าสัมบูรณ์");
        } else if (strncmp(p, "รากที่สอง", strlen("รากที่สอง")) == 0) {
            tokens[tok_count++] = (Token){TOK_SQRT, "รากที่สอง"}; p += strlen("รากที่สอง");
        } else if (strncmp(p, "ฟังก์ชัน", strlen("ฟังก์ชัน")) == 0) {
            tokens[tok_count++] = (Token){TOK_FUNC, "ฟังก์ชัน"}; p += strlen("ฟังก์ชัน");
        } else if (strncmp(p, "ความยาว", strlen("ความยาว")) == 0) {
            tokens[tok_count++] = (Token){TOK_LEN, "ความยาว"}; p += strlen("ความยาว");
        // FIX #2: "ถ้าไม่" MUST come before "ไม่" and "ถ้า"
        } else if (strncmp(p, "ถ้าไม่", strlen("ถ้าไม่")) == 0) {
            tokens[tok_count++] = (Token){TOK_ELSE, "ถ้าไม่"}; p += strlen("ถ้าไม่");
        } else if (strncmp(p, "ตราบใด", strlen("ตราบใด")) == 0) {
            tokens[tok_count++] = (Token){TOK_WHILE, "ตราบใด"}; p += strlen("ตราบใด");
        } else if (strncmp(p, "ทศนิยม", strlen("ทศนิยม")) == 0) {
            tokens[tok_count++] = (Token){TOK_PREC, "ทศนิยม"}; p += strlen("ทศนิยม");
        } else if (strncmp(p, "รับค่า", strlen("รับค่า")) == 0) {
            tokens[tok_count++] = (Token){TOK_INPUT, "รับค่า"}; p += strlen("รับค่า");
        } else if (strncmp(p, "วนลูป", strlen("วนลูป")) == 0) {
            tokens[tok_count++] = (Token){TOK_FOR, "วนลูป"}; p += strlen("วนลูป");
        } else if (strncmp(p, "คืนค่า", strlen("คืนค่า")) == 0) {
            tokens[tok_count++] = (Token){TOK_RETURN, "คืนค่า"}; p += strlen("คืนค่า");
        } else if (strncmp(p, "อาเรย์", strlen("อาเรย์")) == 0) {
            tokens[tok_count++] = (Token){TOK_ARRAY, "อาเรย์"}; p += strlen("อาเรย์");
        } else if (strncmp(p, "ปัดขึ้น", strlen("ปัดขึ้น")) == 0) {
            tokens[tok_count++] = (Token){TOK_CEIL, "ปัดขึ้น"}; p += strlen("ปัดขึ้น");
        } else if (strncmp(p, "ปัดลง", strlen("ปัดลง")) == 0) {
            tokens[tok_count++] = (Token){TOK_FLOOR, "ปัดลง"}; p += strlen("ปัดลง");
        } else if (strncmp(p, "เรียก", strlen("เรียก")) == 0) {
            tokens[tok_count++] = (Token){TOK_CALL, "เรียก"}; p += strlen("เรียก");
        } else if (strncmp(p, "เพิ่ม", strlen("เพิ่ม")) == 0) {
            tokens[tok_count++] = (Token){TOK_PUSH, "เพิ่ม"}; p += strlen("เพิ่ม");
        } else if (strncmp(p, "หยุด", strlen("หยุด")) == 0) {
            tokens[tok_count++] = (Token){TOK_BREAK, "หยุด"}; p += strlen("หยุด");
        } else if (strncmp(p, "แสดง", strlen("แสดง")) == 0) {
            tokens[tok_count++] = (Token){TOK_PRINT, "แสดง"}; p += strlen("แสดง");
        } else if (strncmp(p, "จบ", strlen("จบ")) == 0) {
            tokens[tok_count++] = (Token){TOK_END, "จบ"}; p += strlen("จบ");
        } else if (strncmp(p, "ให้", strlen("ให้")) == 0) {
            tokens[tok_count++] = (Token){TOK_GIVE, "ให้"}; p += strlen("ให้");
        } else if (strncmp(p, "หา", strlen("หา")) == 0) {
            tokens[tok_count++] = (Token){TOK_FIND, "หา"}; p += strlen("หา");
        } else if (strncmp(p, "ถ้า", strlen("ถ้า")) == 0) {
            tokens[tok_count++] = (Token){TOK_IF, "ถ้า"}; p += strlen("ถ้า");
        } else if (strncmp(p, "ข้าม", strlen("ข้าม")) == 0) {
            tokens[tok_count++] = (Token){TOK_CONTINUE, "ข้าม"}; p += strlen("ข้าม");
        } else if (strncmp(p, "ลบ", strlen("ลบ")) == 0) {
            tokens[tok_count++] = (Token){TOK_POP, "ลบ"}; p += strlen("ลบ");
        } else if (strncmp(p, "และ", strlen("และ")) == 0) {
            tokens[tok_count++] = (Token){TOK_AND, "และ"}; p += strlen("และ");
        } else if (strncmp(p, "หรือ", strlen("หรือ")) == 0) {
            tokens[tok_count++] = (Token){TOK_OR, "หรือ"}; p += strlen("หรือ");
        } else if (strncmp(p, "ไม่", strlen("ไม่")) == 0) {
            tokens[tok_count++] = (Token){TOK_NOT, "ไม่"}; p += strlen("ไม่");
        }
        // Operators (check multi-char first)
        else if (strncmp(p, "+=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_PLUSEQ, "+="}; p += 2;
        } else if (strncmp(p, "-=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_MINUSEQ, "-="}; p += 2;
        } else if (strncmp(p, "*=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_MULEQ, "*="}; p += 2;
        } else if (strncmp(p, "/=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_DIVEQ, "/="}; p += 2;
        } else if (strncmp(p, "<=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_LE, "<="}; p += 2;
        } else if (strncmp(p, ">=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_GE, ">="}; p += 2;
        } else if (strncmp(p, "==", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_EQEQ, "=="}; p += 2;
        } else if (strncmp(p, "!=", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_NEQ, "!="}; p += 2;
        } else if (strncmp(p, "**", 2) == 0) {
            tokens[tok_count++] = (Token){TOK_POWER, "**"}; p += 2;
        } else if (*p == '<') {
            tokens[tok_count++] = (Token){TOK_LT, "<"}; p++;
        } else if (*p == '>') {
            tokens[tok_count++] = (Token){TOK_GT, ">"}; p++;
        } else if (*p == '=') {
            tokens[tok_count++] = (Token){TOK_ASSIGN, "="}; p++;
        } else if (*p == '!') {
            tokens[tok_count++] = (Token){TOK_NOT, "!"}; p++;
        } else if (*p == '+') {
            tokens[tok_count++] = (Token){TOK_PLUS, "+"}; p++;
        } else if (*p == '-') {
            tokens[tok_count++] = (Token){TOK_MINUS, "-"}; p++;
        } else if (*p == '*') {
            tokens[tok_count++] = (Token){TOK_MUL, "*"}; p++;
        } else if (*p == '/') {
            tokens[tok_count++] = (Token){TOK_DIV, "/"}; p++;
        } else if (*p == '%') {
            tokens[tok_count++] = (Token){TOK_MOD, "%"}; p++;
        } else if (*p == '(') {
            tokens[tok_count++] = (Token){TOK_LPAREN, "("}; p++;
        } else if (*p == ')') {
            tokens[tok_count++] = (Token){TOK_RPAREN, ")"}; p++;
        } else if (*p == '[') {
            tokens[tok_count++] = (Token){TOK_LBRACKET, "["}; p++;
        } else if (*p == ']') {
            tokens[tok_count++] = (Token){TOK_RBRACKET, "]"}; p++;
        } else if (*p == ',') {
            tokens[tok_count++] = (Token){TOK_COMMA, ","}; p++;
        }
        // String literals
        else if (*p == '"') {
            p++;
            char buf[256];
            int idx = 0;
            while (*p && *p != '"' && idx < 255) {
                buf[idx++] = *p++;
            }
            buf[idx] = '\0';
            if (*p == '"') p++;
            tokens[tok_count].type = TOK_STRING;
            strcpy(tokens[tok_count].text, buf);
            tok_count++;
        }
        // Numbers
        else if (isdigit((unsigned char)*p)) {
            char buf[64];
            int idx = 0;
            while ((isdigit((unsigned char)*p) || *p == '.') && idx < 63) {
                buf[idx++] = *p++;
            }
            buf[idx] = '\0';
            tokens[tok_count].type = TOK_NUM;
            strcpy(tokens[tok_count].text, buf);
            tok_count++;
        }
        // Identifiers
        else {
            char buf[64];
            int idx = 0;
            while (*p && !isspace((unsigned char)*p) && 
                   strchr("=<>!+-*/%()[]," , *p) == NULL && idx < 63) {
                buf[idx++] = *p++;
            }
            buf[idx] = '\0';
            if (strlen(buf) > 0) {
                tokens[tok_count].type = TOK_ID;
                strcpy(tokens[tok_count].text, buf);
                tok_count++;
            }
        }
    }
    tokens[tok_count++] = (Token){TOK_EOF, ""};
}

Token *peek() { return &tokens[tok_pos]; }
Token *next() { return &tokens[tok_pos++]; }

// ==== AST ====

typedef enum { NODE_NUM, NODE_VAR, NODE_BINOP, NODE_UNARY, NODE_ARRAY_ACCESS, NODE_FUNC_CALL } NodeType;

typedef struct Node {
    NodeType type;
    long double fvalue;
    int scale;
    char varname[32];
    char op;
    struct Node *left, *right;
    struct Node *operand;
    struct Node *args[MAX_FUNC_PARAMS];
    int arg_count;
} Node;

Node *make_num_ld(long double v, int scale) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_NUM; n->fvalue = v; n->scale = scale;
    n->left = n->right = n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_var(const char *name) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_VAR; strcpy(n->varname, name);
    n->left = n->right = n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_binop(char op, Node *l, Node *r) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_BINOP; n->op = op;
    n->left = l; n->right = r;
    n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_unary(char op, Node *operand) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_UNARY; n->op = op;
    n->operand = operand;
    n->left = n->right = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_func_call(const char *name) {
    Node *n = malloc(sizeof(Node));
    n->type = NODE_FUNC_CALL;
    strcpy(n->varname, name);
    n->left = n->right = n->operand = NULL;
    n->arg_count = 0;
    return n;
}

// ==== Parser (recursive descent) ====

Node *parse_expr();

Node *parse_factor() {
    Token *t = peek();
    
    // Math functions
    if (t->type == TOK_SQRT || t->type == TOK_ABS || 
        t->type == TOK_FLOOR || t->type == TOK_CEIL) {
        char op = t->type;
        next();
        if (peek()->type == TOK_LPAREN) next();
        Node *operand = parse_expr();
        if (peek()->type == TOK_RPAREN) next();
        return make_unary(op, operand);
    }
    
    // FIX: ความยาว MUST work as a unary operator in expressions
    if (t->type == TOK_LEN) {
        next(); // consume ความยาว
        
        // Handle parentheses: ความยาว(arr)
        if (peek()->type == TOK_LPAREN) {
            next(); // consume (
            if (peek()->type == TOK_ID) {
                Token *arr_name = next();
                Node *n = malloc(sizeof(Node));
                n->type = NODE_UNARY;
                n->op = TOK_LEN;
                n->operand = make_var(arr_name->text);
                n->left = n->right = NULL;
                n->arg_count = 0;
                if (peek()->type == TOK_RPAREN) next(); // consume )
                return n;
            }
        }
        
        // Handle direct identifier: ความยาว arr
        if (peek()->type == TOK_ID) {
            Token *arr_name = next();
            Node *n = malloc(sizeof(Node));
            n->type = NODE_UNARY;
            n->op = TOK_LEN;
            n->operand = make_var(arr_name->text);
            n->left = n->right = NULL;
            n->arg_count = 0;
            return n;
        }
        
        // If nothing follows, return 0
        return make_num_ld(0.0L, 0);
    }
    
    // Unary minus
    if (t->type == TOK_MINUS) {
        next();
        return make_unary('-', parse_factor());
    }
    
    // Parentheses
    if (t->type == TOK_LPAREN) {
        next();
        Node *node = parse_expr();
        if (peek()->type == TOK_RPAREN) next();
        return node;
    }
    
    // Numbers
    if (t->type == TOK_NUM) {
        next();
        Value val = parse_decimal_lexeme(t->text);
        return make_num_ld(val.v, val.scale);
    }
    
    // Variables or array access or function call
    if (t->type == TOK_ID) {
        char name[32];
        strcpy(name, t->text);
        next();
        
        // Check for array access
        if (peek()->type == TOK_LBRACKET) {
            Node *arr = make_var(name);
            arr->type = NODE_ARRAY_ACCESS;
            next(); // consume [
            arr->left = parse_expr(); // index
            if (peek()->type == TOK_RBRACKET) next();
            return arr;
        }
        
        // Check for function call
        if (peek()->type == TOK_LPAREN) {
            next(); // consume (
            Node *func = make_func_call(name);
            while (peek()->type != TOK_RPAREN && peek()->type != TOK_EOF) {
                func->args[func->arg_count++] = parse_expr();
                if (peek()->type == TOK_COMMA) next();
            }
            if (peek()->type == TOK_RPAREN) next();
            return func;
        }
        
        return make_var(name);
    }
    
    return make_num_ld(0.0L, 0);
}

Node *parse_power() {
    Node *node = parse_factor();
    while (peek()->type == TOK_POWER) {
        next();
        node = make_binop('^', node, parse_factor());
    }
    return node;
}

Node *parse_term() {
    Node *node = parse_power();
    while (peek()->type == TOK_MUL || peek()->type == TOK_DIV || peek()->type == TOK_MOD) {
        char op = peek()->text[0];
        next();
        node = make_binop(op, node, parse_power());
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

static Value g_return_value;
static int g_has_return = 0;
static int g_break_flag = 0;
static int g_continue_flag = 0;

static Value eval_node(struct Node *n);

static Value bin_calc(char op, Value l, Value r) {
    Value out;
    switch (op) {
        case '+': out.v = l.v + r.v; out.scale = max_int(l.scale, r.scale); break;
        case '-': out.v = l.v - r.v; out.scale = max_int(l.scale, r.scale); break;
        case '*': out.v = l.v * r.v; out.scale = max_int(l.scale, r.scale); break;
        case '/':
            if (r.v == 0.0L) {
                fprintf(stderr, "Division by zero\n");
                exit(1);
            }
            out.v = l.v / r.v;
            out.scale = max_int(l.scale, r.scale);
            break;
        case '%':
            out.v = fmodl(l.v, r.v);
            out.scale = max_int(l.scale, r.scale);
            break;
        case '^':
            out.v = powl(l.v, r.v);
            out.scale = max_int(l.scale, r.scale);
            break;
        default: out.v = 0.0L; out.scale = 0; break;
    }
    return out;
}

// Forward declaration for function execution
void execute_commands(char **cmds, int count);

static Value eval_node(struct Node *n) {
    if (n == NULL) return make_value(0.0L, 0);
    
    if (n->type == NODE_NUM) return make_value(n->fvalue, n->scale);
    
    if (n->type == NODE_VAR) {
        // Don't check array - just treat everything as potential variable
        // Auto-create variable if it doesn't exist
        if (!has_variable(n->varname)) {
            // Check if it's an array name
            if (get_array(n->varname) != NULL) {
                // It's an array, return 0 (arrays can't be used as values)
                return make_value(0.0L, 0);
            }
            // Not an array and doesn't exist - create it with 0
            set_variable(n->varname, make_value(0.0L, 0));
        }
        return get_variable(n->varname);
    }
    
    if (n->type == NODE_ARRAY_ACCESS) {
        Array *arr = get_array(n->varname);
        if (!arr) {
            // Array doesn't exist - return 0 instead of crashing
            return make_value(0.0L, 0);
        }
        Value idx_val = eval_node(n->left);
        int idx = (int)idx_val.v;
        if (idx < 0 || idx >= arr->size) {
            // Out of bounds - return 0 instead of crashing
            return make_value(0.0L, 0);
        }
        return arr->data[idx];
    }
    
    if (n->type == NODE_FUNC_CALL) {
        Function *func = get_function(n->varname);
        if (!func) {
            // Function not found - return 0 instead of crashing
            return make_value(0.0L, 0);
        }
        
        // Save current variable state
        Variable saved_vars[MAX_VARIABLES];
        int saved_count = var_count;
        memcpy(saved_vars, variables, sizeof(Variable) * var_count);
        
        // Set parameters
        for (int i = 0; i < func->param_count && i < n->arg_count; i++) {
            Value arg_val = eval_node(n->args[i]);
            set_variable(func->params[i], arg_val);
        }
        
        // Execute function body
        int saved_return = g_has_return;
        g_has_return = 0;
        execute_commands(func->body, func->body_count);
        
        Value result = g_has_return ? g_return_value : make_value(0.0L, 0);
        g_has_return = saved_return;
        
        // Restore variable state
        var_count = saved_count;
        memcpy(variables, saved_vars, sizeof(Variable) * saved_count);
        
        return result;
    }
    
    if (n->type == NODE_UNARY) {
        if (n->op == TOK_LEN) {
            // Get array length
            if (n->operand && n->operand->type == NODE_VAR) {
                Array *arr = get_array(n->operand->varname);
                if (arr) {
                    return make_value((long double)arr->size, 0);
                }
                // Not an array - return 0
                return make_value(0.0L, 0);
            }
            return make_value(0.0L, 0);
        }
        
        Value operand = eval_node(n->operand);
        Value out;
        switch (n->op) {
            case '-':
                out.v = -operand.v;
                out.scale = operand.scale;
                break;
            case TOK_SQRT:
                out.v = sqrtl(operand.v);
                out.scale = operand.scale;
                break;
            case TOK_ABS:
                out.v = fabsl(operand.v);
                out.scale = operand.scale;
                break;
            case TOK_FLOOR:
                out.v = floorl(operand.v);
                out.scale = 0;
                break;
            case TOK_CEIL:
                out.v = ceill(operand.v);
                out.scale = 0;
                break;
            default:
                out = operand;
        }
        return out;
    }
    
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
        #ifdef _WIN32
            cmd_buffer[cmd_count] = _strdup(line);
        #else
            cmd_buffer[cmd_count] = strdup(line);
        #endif
        cmd_count++;
    }
}

// helper: evaluate condition with comparison operators and logical operators
int eval_condition_from_tokpos() {
    // FIX #1: Handle NOT operator (fixed duplicate check)
    if (peek()->type == TOK_NOT) {
        next();
        int result = eval_condition_from_tokpos();
        return !result;
    }
    
    Node *left_expr = parse_expr();
    Value left_val = eval_node(left_expr);

    Token *cmp = peek();
    
    // Comparison operators
    if (cmp->type == TOK_LT || cmp->type == TOK_GT || cmp->type == TOK_LE || cmp->type == TOK_GE
        || cmp->type == TOK_EQEQ || cmp->type == TOK_NEQ) {
        TokenType cmpType = cmp->type;
        next();
        Node *right_expr = parse_expr();
        Value right_val = eval_node(right_expr);

        long double L = left_val.v, R = right_val.v;
        int result;
        switch (cmpType) {
            case TOK_LT: result = L < R; break;
            case TOK_GT: result = L > R; break;
            case TOK_LE: result = L <= R; break;
            case TOK_GE: result = L >= R; break;
            case TOK_EQEQ: result = fabsl(L - R) < 1e-10L; break;
            case TOK_NEQ: result = fabsl(L - R) >= 1e-10L; break;
            default: result = 0;
        }
        
        // Check for chained logical operators
        if (peek()->type == TOK_AND) {
            next();
            int right = eval_condition_from_tokpos();
            return result && right;
        }
        
        if (peek()->type == TOK_OR) {
            next();
            int right = eval_condition_from_tokpos();
            return result || right;
        }
        
        return result;
    }
    
    // Logical operators
    if (cmp->type == TOK_AND) {
        next();
        int right = eval_condition_from_tokpos();
        return (left_val.v != 0.0L) && right;
    }
    
    if (cmp->type == TOK_OR) {
        next();
        int right = eval_condition_from_tokpos();
        return (left_val.v != 0.0L) || right;
    }
    
    // Just a value expression (truthy check)
    return left_val.v != 0.0L;
}

// parse user input string to Value (preserve trailing zeros)
static Value parse_input_value() {
    char buf[256];
    // FIX #8: Handle EOF properly - don't silently return 0
    if (!fgets(buf, sizeof(buf), stdin)) {
        // Return 0 but could be enhanced to return NIL/error
        fprintf(stderr, "(EOF reached)\n");
        return make_value(0.0L, 0);
    }
    buf[strcspn(buf, "\r\n")] = '\0';
    char *s = buf;
    while (*s && isspace((unsigned char)*s)) s++;
    if (*s == '\0') return make_value(0.0L, 0);

    int scale = 0;
    char *dot = strchr(s, '.');
    if (dot) scale = (int)strlen(dot + 1);

    long double v = strtold(s, NULL);
    return make_value(v, scale);
}

void execute_commands(char **cmds, int count) {
    for (int i = 0; i < count && !g_has_return && !g_break_flag; i++) {
        if (g_continue_flag) {
            g_continue_flag = 0;
            continue;
        }
        
        lex_line(cmds[i]);
        
        // Skip empty lines
        if (tok_count <= 1) continue;
        
        // Skip "จบ" (END) statements - they're just markers
        if (tokens[0].type == TOK_END) {
            continue;
        }
        
        // FUNCTION DEFINITION
        if (tokens[0].type == TOK_FUNC) {
            tok_pos = 1;
            Token *func_name = next();
            
            char params[MAX_FUNC_PARAMS][32];
            int param_count = 0;
            
            if (peek()->type == TOK_LPAREN) {
                next();
                while (peek()->type != TOK_RPAREN && peek()->type != TOK_EOF) {
                    Token *param = next();
                    if (param->type == TOK_ID) {
                        strcpy(params[param_count++], param->text);
                    }
                    if (peek()->type == TOK_COMMA) next();
                }
                if (peek()->type == TOK_RPAREN) next();
            }
            
            // Collect function body
            char *body[MAX_CMDS];
            int body_count = 0;
            i++;
            
            // FIX #3: Use ONLY token type, remove strstr check
            int depth = 1;
            while (i < count) {
                lex_line(cmds[i]);
                
                if (tokens[0].type == TOK_FUNC || tokens[0].type == TOK_IF || 
                    tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR) {
                    depth++;
                }
                
                // FIX #3: ONLY check token type, NOT string content
                if (tokens[0].type == TOK_END) {
                    depth--;
                    if (depth == 0) break;
                }
                
                body[body_count++] = cmds[i];
                i++;
            }
            
            add_function(func_name->text, params, param_count, body, body_count);
            continue;
        }
        
        // RETURN
        if (tokens[0].type == TOK_RETURN) {
            tok_pos = 1;
            if (peek()->type != TOK_EOF && peek()->type != TOK_END) {
                Node *expr = parse_expr();
                g_return_value = eval_node(expr);
            } else {
                g_return_value = make_value(0.0L, 0);
            }
            g_has_return = 1;
            return;
        }
        
        // BREAK
        if (tokens[0].type == TOK_BREAK) {
            g_break_flag = 1;
            return;
        }
        
        // CONTINUE
        if (tokens[0].type == TOK_CONTINUE) {
            g_continue_flag = 1;
            continue;
        }
        
        // WHILE LOOP
        if (tokens[0].type == TOK_WHILE) {
            int start_line = i;
            char **loop_body = malloc(sizeof(char*) * MAX_CMDS);
            int loop_body_count = 0;
            
            // Collect loop body
            i++;
            int depth = 1;
            while (i < count) {
                lex_line(cmds[i]);
                if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                    tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC) {
                    depth++;
                }
                // FIX #3: Remove strstr check
                if (tokens[0].type == TOK_END) {
                    depth--;
                    if (depth == 0) break;
                }
                loop_body[loop_body_count++] = cmds[i];
                i++;
            }
            
            //  FIX #6: Save and restore token state for each loop iteration
            while (1) {
                // Save current token state
                int saved_tok_count = tok_count;
                int saved_tok_pos = tok_pos;
                Token saved_tokens[256];
                memcpy(saved_tokens, tokens, sizeof(tokens));
                
                // Parse condition
                lex_line(cmds[start_line]);
                tok_pos = 1;
                int cond = eval_condition_from_tokpos();
                
                // Restore token state
                tok_count = saved_tok_count;
                tok_pos = saved_tok_pos;
                memcpy(tokens, saved_tokens, sizeof(tokens));
                
                if (!cond || g_break_flag) {
                    g_break_flag = 0;
                    break;
                }
                
                execute_commands(loop_body, loop_body_count);
                if (g_has_return) break;
            }
            
            free(loop_body);
            continue;
        }
        
        // FOR LOOP: วนลูป i = 1 ถึง 10
        if (tokens[0].type == TOK_FOR) {
            tok_pos = 1;
            Token *loop_var = next();
            
            // Skip '=' if present
            if (peek()->type == TOK_ASSIGN) next();
            
            Node *start_expr = parse_expr();
            Value start_val = eval_node(start_expr);
            
            // Skip "ถึง" keyword
            while (peek()->type != TOK_NUM && peek()->type != TOK_ID && 
                   peek()->type != TOK_LPAREN && peek()->type != TOK_MINUS &&
                   peek()->type != TOK_EOF) {
                next();
            }
            
            Node *end_expr = parse_expr();
            Value end_val = eval_node(end_expr);
            
            // Collect loop body
            char **loop_body = malloc(sizeof(char*) * MAX_CMDS);
            int loop_body_count = 0;
            i++;
            int depth = 1;
            while (i < count) {
                lex_line(cmds[i]);
                if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                    tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC) {
                    depth++;
                }
                //FIX #3: Remove strstr
                if (tokens[0].type == TOK_END) {
                    depth--;
                    if (depth == 0) break;
                }
                loop_body[loop_body_count++] = cmds[i];
                i++;
            }
            
            // Execute loop - CREATE loop variable before executing body
            for (long double v = start_val.v; v <= end_val.v; v += 1.0L) {
                set_variable(loop_var->text, make_value(v, 0));
                
                if (g_break_flag) {
                    g_break_flag = 0;
                    break;
                }
                
                execute_commands(loop_body, loop_body_count);
                if (g_has_return) break;
            }
            
            free(loop_body);
            continue;
        }
        
        // IF statement
        if (tokens[0].type == TOK_IF) {
            tok_pos = 1;
            int cond = eval_condition_from_tokpos();
            
            char **if_body = malloc(sizeof(char*) * MAX_CMDS);
            int if_body_count = 0;
            i++;
            int depth = 1;
            while (i < count) {
                lex_line(cmds[i]);
                
                if (depth == 1 && tokens[0].type == TOK_ELSE) break;
                
                if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                    tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC) {
                    depth++;
                }
                //FIX #3: Remove strstr
                if (tokens[0].type == TOK_END) {
                    depth--;
                    if (depth == 0) break;
                }
                if_body[if_body_count++] = cmds[i];
                i++;
            }
            
            char **else_body = malloc(sizeof(char*) * MAX_CMDS);
            int else_body_count = 0;
            // ✅ FIX #3: Remove strstr for else check too
            if (i < count && tokens[0].type == TOK_ELSE) {
                i++;
                depth = 1;
                while (i < count) {
                    lex_line(cmds[i]);
                    if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                        tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC) {
                        depth++;
                    }
                    if (tokens[0].type == TOK_END) {
                        depth--;
                        if (depth == 0) break;
                    }
                    else_body[else_body_count++] = cmds[i];
                    i++;
                }
            }
            
            if (cond) {
                execute_commands(if_body, if_body_count);
            } else {
                execute_commands(else_body, else_body_count);
            }
            
            free(if_body);
            free(else_body);
            continue;
        }
        
        // GIVE with compound assignment
        if (tokens[0].type == TOK_GIVE) {
            tok_pos = 1;
            
            if (peek()->type == TOK_ID) {
                char var_name[32];
                strcpy(var_name, peek()->text);
                next();
                
                if (peek()->type == TOK_LBRACKET) {
                    // Array element assignment
                    next(); // [
                    Node *index_expr = parse_expr();
                    Value index_val = eval_node(index_expr);
                    if (peek()->type == TOK_RBRACKET) next();
                    if (peek()->type == TOK_ASSIGN) next();
                    Node *value_expr = parse_expr();
                    Value value = eval_node(value_expr);
                    array_set(var_name, (int)index_val.v, value);
                } else {
                    Token *op = peek();
                    
                    if (op->type == TOK_ASSIGN) {
                        next();
                        Node *expr = parse_expr();
                        set_variable(var_name, eval_node(expr));
                    } else if (op->type == TOK_PLUSEQ) {
                        next();
                        Node *expr = parse_expr();
                        // Create variable with 0 if it doesn't exist
                        if (!has_variable(var_name)) {
                            set_variable(var_name, make_value(0.0L, 0));
                        }
                        Value current = get_variable(var_name);
                        Value delta = eval_node(expr);
                        set_variable(var_name, bin_calc('+', current, delta));
                    } else if (op->type == TOK_MINUSEQ) {
                        next();
                        Node *expr = parse_expr();
                        if (!has_variable(var_name)) {
                            set_variable(var_name, make_value(0.0L, 0));
                        }
                        Value current = get_variable(var_name);
                        Value delta = eval_node(expr);
                        set_variable(var_name, bin_calc('-', current, delta));
                    } else if (op->type == TOK_MULEQ) {
                        next();
                        Node *expr = parse_expr();
                        if (!has_variable(var_name)) {
                            set_variable(var_name, make_value(0.0L, 0));
                        }
                        Value current = get_variable(var_name);
                        Value delta = eval_node(expr);
                        set_variable(var_name, bin_calc('*', current, delta));
                    } else if (op->type == TOK_DIVEQ) {
                        next();
                        Node *expr = parse_expr();
                        if (!has_variable(var_name)) {
                            set_variable(var_name, make_value(1.0L, 0));
                        }
                        Value current = get_variable(var_name);
                        Value delta = eval_node(expr);
                        set_variable(var_name, bin_calc('/', current, delta));
                    }
                }
            }
            continue;
        }
        // FIND (print)
        else if (tokens[0].type == TOK_FIND) {
            tok_pos = 1;
            Node *expr = parse_expr();
            print_value(eval_node(expr));
            continue;
        }
        // PRINT (without newline)
        else if (tokens[0].type == TOK_PRINT) {
            tok_pos = 1;
            if (peek()->type == TOK_STRING) {
                printf("%s", next()->text);
            } else {
                Node *expr = parse_expr();
                Value v = eval_node(expr);
                int decimals = g_fixed_dp ? g_out_dp : (v.scale > 0 ? v.scale : 0);
                char fmt[16];
                snprintf(fmt, sizeof(fmt), "%%.%dLf", decimals);
                printf(fmt, round_to(v.v, decimals));
            }
            fflush(stdout);
            continue;
        }
        // PRINTLN
        else if (tokens[0].type == TOK_PRINTLN) {
            tok_pos = 1;
            if (peek()->type == TOK_STRING) {
                printf("%s\n", next()->text);
            } else {
                Node *expr = parse_expr();
                print_value(eval_node(expr));
            }
            continue;
        }
        // INPUT
        else if (tokens[0].type == TOK_INPUT) {
            tok_pos = 1;
            Token *var = next();
            printf("กรอกค่า %s: ", var->text);
            fflush(stdout);
            Value input = parse_input_value();
            set_variable(var->text, input);
            continue;
        }
        // ARRAY creation
        else if (tokens[0].type == TOK_ARRAY) {
            tok_pos = 1;
            Token *name = next();
            if (peek()->type == TOK_LBRACKET) next();
            Node *size_expr = parse_expr();
            Value size_val = eval_node(size_expr);
            create_array(name->text, (int)size_val.v);
            if (peek()->type == TOK_RBRACKET) next();
            continue;
        }
        // ARRAY PUSH
        else if (tokens[0].type == TOK_PUSH) {
            tok_pos = 1;
            Token *arr_name = next();
            Node *value_expr = parse_expr();
            Value val = eval_node(value_expr);
            array_push(arr_name->text, val);
            continue;
        }
        // ARRAY POP
        else if (tokens[0].type == TOK_POP) {
            tok_pos = 1;
            Token *arr_name = next();
            array_pop(arr_name->text);
            continue;
        }
        // CALL function
        else if (tokens[0].type == TOK_CALL) {
            tok_pos = 1;
            Token *func_name = next();
            
            Node *func_call = make_func_call(func_name->text);
            
            if (peek()->type == TOK_LPAREN) {
                next();
                while (peek()->type != TOK_RPAREN && peek()->type != TOK_EOF) {
                    func_call->args[func_call->arg_count++] = parse_expr();
                    if (peek()->type == TOK_COMMA) next();
                }
                if (peek()->type == TOK_RPAREN) next();
            }
            
            eval_node(func_call);
            continue;
        }
        // PREC (decimal precision)
        else if (tokens[0].type == TOK_PREC) {
            tok_pos = 1;
            Node *expr = parse_expr();
            Value v = eval_node(expr);
            g_out_dp = (int)v.v;
            g_fixed_dp = 1;
            continue;
        }
    }
}

void run_all_commands() {
    execute_commands(cmd_buffer, cmd_count);
    cmd_count = 0;
}

// ==== Helpers ====

static void ltrim(char *str) {
    int index = 0;
    while (str[index] == ' ' || str[index] == '\t') index++;
    if (index > 0) {
        int i = 0;
        while (str[index]) {
            str[i++] = str[index++];
        }
        str[i] = '\0';
    }
}

// ==== Shell Interpreter ====

void __Ark_Shell() {
    char line[256];
    setlocale(LC_NUMERIC, "C");
    printf("Aeroki Shell Mode (type 'ออก' to exit)\n");
    while (1) {
        printf(">>> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\r\n")] = '\0';
        ltrim(line);
        
        if (strlen(line) == 0) continue;
        if (strcmp(line, "ออก") == 0) break;
        
        add_command(line);
        run_all_commands();
    }
}

// ==== File Interpreter ====

void __Ark_Interpreted(FILE *src) {
    char line[256];
    setlocale(LC_NUMERIC, "C");

    while (fgets(line, sizeof(line), src)) {
        line[strcspn(line, "\r\n")] = '\0';
        ltrim(line);
        if (strlen(line) == 0) continue;
        if (line[0] == '#') continue; // comments
        add_command(line);
    }
    run_all_commands();
}
