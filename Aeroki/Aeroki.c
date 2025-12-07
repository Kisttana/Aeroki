#include "Aeroki.h"

#define MAX_VARIABLES 500
#define MAX_CMDS 2000
#define MAX_ARRAYS 100
#define MAX_ARRAY_SIZE 5000
#define MAX_FUNCTIONS 200
#define MAX_FUNC_PARAMS 30
#define MAX_TOKEN_LEN 256

typedef struct
{
    long double v;
    int scale;
} Value;

typedef struct
{
    char name[64];
    Value value;
    int depth;
} Variable;

typedef struct
{
    char name[64];
    Value data[MAX_ARRAY_SIZE];
    int size;
    int capacity;
} Array;

typedef struct
{
    char name[64];
    char params[MAX_FUNC_PARAMS][64];
    int param_count;
    char *body[MAX_CMDS];
    int body_count;
} Function;

typedef enum
{
    TOK_UNKNOWN,
    TOK_GIVE,
    TOK_FIND,
    TOK_INPUT,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_TO,
    TOK_BREAK,
    TOK_CONTINUE,
    TOK_PREC,
    TOK_FUNC,
    TOK_RETURN,
    TOK_CALL,
    TOK_END,
    TOK_PRINT,
    TOK_PRINTLN,
    TOK_ARRAY,
    TOK_PUSH,
    TOK_POP,
    TOK_LEN,
    TOK_SQRT,
    TOK_ABS,
    TOK_FLOOR,
    TOK_CEIL,
    TOK_AND,
    TOK_OR,
    TOK_NOT,
    TOK_ID,
    TOK_NUM,
    TOK_STRING,
    TOK_ASSIGN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,
    TOK_POWER,
    TOK_PLUSEQ,
    TOK_MINUSEQ,
    TOK_MULEQ,
    TOK_DIVEQ,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_EQEQ,
    TOK_NEQ,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACKET,
    TOK_RBRACKET,
    TOK_COMMA,
    TOK_EOF
} TokenType;

typedef struct
{
    TokenType type;
    char text[MAX_TOKEN_LEN];
} Token;

typedef enum
{
    NODE_NUM,
    NODE_VAR,
    NODE_BINOP,
    NODE_UNARY,
    NODE_ARRAY_ACCESS,
    NODE_FUNC_CALL
} NodeType;

typedef struct Node
{
    NodeType type;
    long double fvalue;
    int scale;
    char varname[64];
    char op;
    struct Node *left;
    struct Node *right;
    struct Node *operand;
    struct Node *args[MAX_FUNC_PARAMS];
    int arg_count;
} Node;

Variable variables[MAX_VARIABLES];
int var_count = 0;
int current_scope_depth = 0;

Array arrays[MAX_ARRAYS];
int array_count = 0;

Function functions[MAX_FUNCTIONS];
int func_count = 0;

Token tokens[MAX_CMDS];
int tok_count = 0;
int tok_pos = 0;

char *cmd_buffer[MAX_CMDS];
int cmd_count = 0;

static int g_out_dp = 2;
static int g_fixed_dp = 0;

static Value g_return_value;
static int g_has_return = 0;
static int g_break_flag = 0;
static int g_continue_flag = 0;

static inline Value make_value(long double v, int scale)
{
    Value x;
    x.v = v;
    x.scale = (scale < 0 ? 0 : scale);
    return x;
}

static inline int max_int(int a, int b)
{
    return a > b ? a : b;
}

static void ltrim(char *str)
{
    int index = 0;
    while (str[index] == ' ' || str[index] == '\t')
    {
        index++;
    }

    if (index > 0)
    {
        int i = 0;
        while (str[index])
        {
            str[i++] = str[index++];
        }
        str[i] = '\0';
    }
}

static long double round_to(long double x, int dp)
{
    if (dp < 0)
    {
        dp = 0;
    }

    long double p = powl(10.0L, (long double)dp);
    long double y = x * p;
    long double eps = fabsl(y) * 1e-15L + 1e-18L;

    if (y >= 0)
    {
        y += eps;
    }
    else
    {
        y -= eps;
    }

    long double r = roundl(y);
    return r / p;
}

static void print_value(Value val, int newline)
{
    int decimals;

    if (g_fixed_dp)
    {
        decimals = g_out_dp;
    }
    else
    {
        if (val.scale > 0)
        {
            decimals = val.scale;
        }
        else
        {
            long double int_part;
            if (modfl(val.v, &int_part) == 0.0)
            {
                decimals = 0;
            }
            else
            {
                decimals = 2;
            }
        }
    }

    long double rv = round_to(val.v, decimals);
    char fmt[32];

    if (newline)
    {
        snprintf(fmt, sizeof(fmt), "%%.%dLf\n", decimals);
    }
    else
    {
        snprintf(fmt, sizeof(fmt), "%%.%dLf", decimals);
    }

    printf(fmt, rv);

    if (!newline)
    {
        fflush(stdout);
    }
}

static Value parse_decimal_lexeme(const char *lex)
{
    int scale = 0;
    const char *dot = strchr(lex, '.');

    if (dot)
    {
        scale = (int)strlen(dot + 1);
    }

    long double v = strtold(lex, NULL);
    return make_value(v, scale);
}

int find_variable_index(const char *name)
{
    for (int i = var_count - 1; i >= 0; --i)
    {
        if (strcmp(variables[i].name, name) == 0)
        {
            return i;
        }
    }
    return -1;
}

Value get_variable(const char *name)
{
    int idx = find_variable_index(name);
    if (idx != -1)
    {
        return variables[idx].value;
    }
    return make_value(0.0L, 0);
}

void set_variable(const char *name, Value value)
{
    int idx = find_variable_index(name);

    if (idx != -1)
    {
        variables[idx].value = value;
        return;
    }

    if (var_count < MAX_VARIABLES)
    {
        strcpy(variables[var_count].name, name);
        variables[var_count].value = value;
        variables[var_count].depth = current_scope_depth;
        var_count++;
    }
    else
    {
        fprintf(stderr, "Error: Too many variables declared.\n");
        exit(1);
    }
}

void clear_variables_by_depth(int depth)
{
    while (var_count > 0 && variables[var_count - 1].depth >= depth)
    {
        var_count--;
    }
}

Array* get_array(const char *name)
{
    for (int i = 0; i < array_count; i++)
    {
        if (strcmp(arrays[i].name, name) == 0)
        {
            return &arrays[i];
        }
    }
    return NULL;
}

void create_array(const char *name, int size)
{
    if (array_count >= MAX_ARRAYS)
    {
        fprintf(stderr, "Error: Too many arrays declared.\n");
        return;
    }

    strcpy(arrays[array_count].name, name);
    arrays[array_count].size = 0;
    arrays[array_count].capacity = size > 0 ? size : MAX_ARRAY_SIZE;

    for (int i = 0; i < arrays[array_count].capacity; i++)
    {
        arrays[array_count].data[i] = make_value(0.0L, 0);
    }

    array_count++;
}

void array_push(const char *name, Value val)
{
    Array *arr = get_array(name);
    if (!arr) return;

    if (arr->size >= arr->capacity)
    {
        fprintf(stderr, "Error: Array '%s' is full.\n", name);
        return;
    }

    arr->data[arr->size++] = val;
}

void array_pop(const char *name)
{
    Array *arr = get_array(name);
    if (!arr || arr->size == 0) return;

    arr->size--;
}

void array_set(const char *name, int index, Value val)
{
    Array *arr = get_array(name);
    if (!arr) return;

    if (index >= arr->capacity)
    {
        fprintf(stderr, "Error: Index out of bounds for array '%s'.\n", name);
        return;
    }

    if (index >= arr->size)
    {
        for (int i = arr->size; i < index; i++)
        {
            arr->data[i] = make_value(0.0L, 0);
        }
        arr->size = index + 1;
    }

    arr->data[index] = val;
}

Function* get_function(const char *name)
{
    for (int i = 0; i < func_count; i++)
    {
        if (strcmp(functions[i].name, name) == 0)
        {
            return &functions[i];
        }
    }
    return NULL;
}

void add_function(const char *name, char params[][64], int param_count, char **body, int body_count)
{
    if (func_count >= MAX_FUNCTIONS)
    {
        fprintf(stderr, "Error: Too many functions defined.\n");
        return;
    }

    strcpy(functions[func_count].name, name);
    functions[func_count].param_count = param_count;

    for (int i = 0; i < param_count; i++)
    {
        strcpy(functions[func_count].params[i], params[i]);
    }

    functions[func_count].body_count = body_count;

    for (int i = 0; i < body_count; i++)
    {
        #ifdef _WIN32
            functions[func_count].body[i] = _strdup(body[i]);
        #else
            functions[func_count].body[i] = strdup(body[i]);
        #endif
    }

    func_count++;
}

Token *peek()
{
    return &tokens[tok_pos];
}

Token *next()
{
    return &tokens[tok_pos++];
}

void lex_line(const char *line)
{
    tok_count = 0;
    tok_pos = 0;
    const char *p = line;

    while (*p)
    {
        if (isspace((unsigned char)*p))
        {
            p++;
            continue;
        }

        if (strncmp(p, "แสดงบรรทัด", strlen("แสดงบรรทัด")) == 0)
        {
            tokens[tok_count].type = TOK_PRINTLN;
            strcpy(tokens[tok_count].text, "แสดงบรรทัด");
            tok_count++;
            p += strlen("แสดงบรรทัด");
        }
        else if (strncmp(p, "ค่าสัมบูรณ์", strlen("ค่าสัมบูรณ์")) == 0)
        {
            tokens[tok_count].type = TOK_ABS;
            strcpy(tokens[tok_count].text, "ค่าสัมบูรณ์");
            tok_count++;
            p += strlen("ค่าสัมบูรณ์");
        }
        else if (strncmp(p, "รากที่สอง", strlen("รากที่สอง")) == 0)
        {
            tokens[tok_count].type = TOK_SQRT;
            strcpy(tokens[tok_count].text, "รากที่สอง");
            tok_count++;
            p += strlen("รากที่สอง");
        }
        else if (strncmp(p, "ฟังก์ชัน", strlen("ฟังก์ชัน")) == 0)
        {
            tokens[tok_count].type = TOK_FUNC;
            strcpy(tokens[tok_count].text, "ฟังก์ชัน");
            tok_count++;
            p += strlen("ฟังก์ชัน");
        }
        else if (strncmp(p, "ความยาว", strlen("ความยาว")) == 0)
        {
            tokens[tok_count].type = TOK_LEN;
            strcpy(tokens[tok_count].text, "ความยาว");
            tok_count++;
            p += strlen("ความยาว");
        }
        else if (strncmp(p, "ถ้าไม่", strlen("ถ้าไม่")) == 0)
        {
            tokens[tok_count].type = TOK_ELSE;
            strcpy(tokens[tok_count].text, "ถ้าไม่");
            tok_count++;
            p += strlen("ถ้าไม่");
        }
        else if (strncmp(p, "ขณะที่", strlen("ขณะที่")) == 0)
        {
            tokens[tok_count].type = TOK_WHILE;
            strcpy(tokens[tok_count].text, "ขณะที่");
            tok_count++;
            p += strlen("ขณะที่");
        }
        else if (strncmp(p, "ตราบใด", strlen("ตราบใด")) == 0)
        {
            tokens[tok_count].type = TOK_WHILE;
            strcpy(tokens[tok_count].text, "ตราบใด");
            tok_count++;
            p += strlen("ตราบใด");
        }
        else if (strncmp(p, "ทศนิยม", strlen("ทศนิยม")) == 0)
        {
            tokens[tok_count].type = TOK_PREC;
            strcpy(tokens[tok_count].text, "ทศนิยม");
            tok_count++;
            p += strlen("ทศนิยม");
        }
        else if (strncmp(p, "รับค่า", strlen("รับค่า")) == 0)
        {
            tokens[tok_count].type = TOK_INPUT;
            strcpy(tokens[tok_count].text, "รับค่า");
            tok_count++;
            p += strlen("รับค่า");
        }
        else if (strncmp(p, "วนลูป", strlen("วนลูป")) == 0)
        {
            tokens[tok_count].type = TOK_FOR;
            strcpy(tokens[tok_count].text, "วนลูป");
            tok_count++;
            p += strlen("วนลูป");
        }
        else if (strncmp(p, "คืนค่า", strlen("คืนค่า")) == 0)
        {
            tokens[tok_count].type = TOK_RETURN;
            strcpy(tokens[tok_count].text, "คืนค่า");
            tok_count++;
            p += strlen("คืนค่า");
        }
        else if (strncmp(p, "อาเรย์", strlen("อาเรย์")) == 0)
        {
            tokens[tok_count].type = TOK_ARRAY;
            strcpy(tokens[tok_count].text, "อาเรย์");
            tok_count++;
            p += strlen("อาเรย์");
        }
        else if (strncmp(p, "ปัดขึ้น", strlen("ปัดขึ้น")) == 0)
        {
            tokens[tok_count].type = TOK_CEIL;
            strcpy(tokens[tok_count].text, "ปัดขึ้น");
            tok_count++;
            p += strlen("ปัดขึ้น");
        }
        else if (strncmp(p, "ปัดลง", strlen("ปัดลง")) == 0)
        {
            tokens[tok_count].type = TOK_FLOOR;
            strcpy(tokens[tok_count].text, "ปัดลง");
            tok_count++;
            p += strlen("ปัดลง");
        }
        else if (strncmp(p, "เรียก", strlen("เรียก")) == 0)
        {
            tokens[tok_count].type = TOK_CALL;
            strcpy(tokens[tok_count].text, "เรียก");
            tok_count++;
            p += strlen("เรียก");
        }
        else if (strncmp(p, "เพิ่ม", strlen("เพิ่ม")) == 0)
        {
            tokens[tok_count].type = TOK_PUSH;
            strcpy(tokens[tok_count].text, "เพิ่ม");
            tok_count++;
            p += strlen("เพิ่ม");
        }
        else if (strncmp(p, "หยุด", strlen("หยุด")) == 0)
        {
            tokens[tok_count].type = TOK_BREAK;
            strcpy(tokens[tok_count].text, "หยุด");
            tok_count++;
            p += strlen("หยุด");
        }
        else if (strncmp(p, "แสดง", strlen("แสดง")) == 0)
        {
            tokens[tok_count].type = TOK_PRINT;
            strcpy(tokens[tok_count].text, "แสดง");
            tok_count++;
            p += strlen("แสดง");
        }
        else if (strncmp(p, "ถึง", strlen("ถึง")) == 0)
        {
            tokens[tok_count].type = TOK_TO;
            strcpy(tokens[tok_count].text, "ถึง");
            tok_count++;
            p += strlen("ถึง");
        }
        else if (strncmp(p, "จบ", strlen("จบ")) == 0)
        {
            tokens[tok_count].type = TOK_END;
            strcpy(tokens[tok_count].text, "จบ");
            tok_count++;
            p += strlen("จบ");
        }
        else if (strncmp(p, "ให้", strlen("ให้")) == 0)
        {
            tokens[tok_count].type = TOK_GIVE;
            strcpy(tokens[tok_count].text, "ให้");
            tok_count++;
            p += strlen("ให้");
        }
        else if (strncmp(p, "หา", strlen("หา")) == 0)
        {
            tokens[tok_count].type = TOK_FIND;
            strcpy(tokens[tok_count].text, "หา");
            tok_count++;
            p += strlen("หา");
        }
        else if (strncmp(p, "ถ้า", strlen("ถ้า")) == 0)
        {
            tokens[tok_count].type = TOK_IF;
            strcpy(tokens[tok_count].text, "ถ้า");
            tok_count++;
            p += strlen("ถ้า");
        }
        else if (strncmp(p, "ข้าม", strlen("ข้าม")) == 0)
        {
            tokens[tok_count].type = TOK_CONTINUE;
            strcpy(tokens[tok_count].text, "ข้าม");
            tok_count++;
            p += strlen("ข้าม");
        }
        else if (strncmp(p, "ลบ", strlen("ลบ")) == 0)
        {
            tokens[tok_count].type = TOK_POP;
            strcpy(tokens[tok_count].text, "ลบ");
            tok_count++;
            p += strlen("ลบ");
        }
        else if (strncmp(p, "และ", strlen("และ")) == 0)
        {
            tokens[tok_count].type = TOK_AND;
            strcpy(tokens[tok_count].text, "และ");
            tok_count++;
            p += strlen("และ");
        }
        else if (strncmp(p, "หรือ", strlen("หรือ")) == 0)
        {
            tokens[tok_count].type = TOK_OR;
            strcpy(tokens[tok_count].text, "หรือ");
            tok_count++;
            p += strlen("หรือ");
        }
        else if (strncmp(p, "ไม่", strlen("ไม่")) == 0)
        {
            tokens[tok_count].type = TOK_NOT;
            strcpy(tokens[tok_count].text, "ไม่");
            tok_count++;
            p += strlen("ไม่");
        }
        else if (strncmp(p, "+=", 2) == 0)
        {
            tokens[tok_count].type = TOK_PLUSEQ;
            strcpy(tokens[tok_count].text, "+=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "-=", 2) == 0)
        {
            tokens[tok_count].type = TOK_MINUSEQ;
            strcpy(tokens[tok_count].text, "-=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "*=", 2) == 0)
        {
            tokens[tok_count].type = TOK_MULEQ;
            strcpy(tokens[tok_count].text, "*=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "/=", 2) == 0)
        {
            tokens[tok_count].type = TOK_DIVEQ;
            strcpy(tokens[tok_count].text, "/=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "<=", 2) == 0)
        {
            tokens[tok_count].type = TOK_LE;
            strcpy(tokens[tok_count].text, "<=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, ">=", 2) == 0)
        {
            tokens[tok_count].type = TOK_GE;
            strcpy(tokens[tok_count].text, ">=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "==", 2) == 0)
        {
            tokens[tok_count].type = TOK_EQEQ;
            strcpy(tokens[tok_count].text, "==");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "!=", 2) == 0)
        {
            tokens[tok_count].type = TOK_NEQ;
            strcpy(tokens[tok_count].text, "!=");
            tok_count++;
            p += 2;
        }
        else if (strncmp(p, "**", 2) == 0)
        {
            tokens[tok_count].type = TOK_POWER;
            strcpy(tokens[tok_count].text, "**");
            tok_count++;
            p += 2;
        }
        else if (*p == '<')
        {
            tokens[tok_count].type = TOK_LT;
            strcpy(tokens[tok_count].text, "<");
            tok_count++;
            p++;
        }
        else if (*p == '>')
        {
            tokens[tok_count].type = TOK_GT;
            strcpy(tokens[tok_count].text, ">");
            tok_count++;
            p++;
        }
        else if (*p == '=')
        {
            tokens[tok_count].type = TOK_ASSIGN;
            strcpy(tokens[tok_count].text, "=");
            tok_count++;
            p++;
        }
        else if (*p == '!')
        {
            tokens[tok_count].type = TOK_NOT;
            strcpy(tokens[tok_count].text, "!");
            tok_count++;
            p++;
        }
        else if (*p == '+')
        {
            tokens[tok_count].type = TOK_PLUS;
            strcpy(tokens[tok_count].text, "+");
            tok_count++;
            p++;
        }
        else if (*p == '-')
        {
            tokens[tok_count].type = TOK_MINUS;
            strcpy(tokens[tok_count].text, "-");
            tok_count++;
            p++;
        }
        else if (*p == '*')
        {
            tokens[tok_count].type = TOK_MUL;
            strcpy(tokens[tok_count].text, "*");
            tok_count++;
            p++;
        }
        else if (*p == '/')
        {
            tokens[tok_count].type = TOK_DIV;
            strcpy(tokens[tok_count].text, "/");
            tok_count++;
            p++;
        }
        else if (*p == '%')
        {
            tokens[tok_count].type = TOK_MOD;
            strcpy(tokens[tok_count].text, "%");
            tok_count++;
            p++;
        }
        else if (*p == '(')
        {
            tokens[tok_count].type = TOK_LPAREN;
            strcpy(tokens[tok_count].text, "(");
            tok_count++;
            p++;
        }
        else if (*p == ')')
        {
            tokens[tok_count].type = TOK_RPAREN;
            strcpy(tokens[tok_count].text, ")");
            tok_count++;
            p++;
        }
        else if (*p == '[')
        {
            tokens[tok_count].type = TOK_LBRACKET;
            strcpy(tokens[tok_count].text, "[");
            tok_count++;
            p++;
        }
        else if (*p == ']')
        {
            tokens[tok_count].type = TOK_RBRACKET;
            strcpy(tokens[tok_count].text, "]");
            tok_count++;
            p++;
        }
        else if (*p == ',')
        {
            tokens[tok_count].type = TOK_COMMA;
            strcpy(tokens[tok_count].text, ",");
            tok_count++;
            p++;
        }
        else if (*p == '"')
        {
            p++;
            char buf[256];
            int idx = 0;
            while (*p && *p != '"' && idx < 255)
            {
                buf[idx++] = *p++;
            }
            buf[idx] = '\0';
            
            if (*p == '"') p++;
            
            tokens[tok_count].type = TOK_STRING;
            strcpy(tokens[tok_count].text, buf);
            tok_count++;
        }
        else if (isdigit((unsigned char)*p))
        {
            char buf[64];
            int idx = 0;
            while ((isdigit((unsigned char)*p) || *p == '.') && idx < 63)
            {
                buf[idx++] = *p++;
            }
            buf[idx] = '\0';
            
            tokens[tok_count].type = TOK_NUM;
            strcpy(tokens[tok_count].text, buf);
            tok_count++;
        }
        else
        {
            char buf[64];
            int idx = 0;
            while (*p && !isspace((unsigned char)*p) && 
                   strchr("=<>!+-*/%()[]," , *p) == NULL && idx < 63)
            {
                buf[idx++] = *p++;
            }
            buf[idx] = '\0';
            
            if (strlen(buf) > 0)
            {
                if (strcmp(buf, "จบ") == 0) tokens[tok_count].type = TOK_END;
                else if (strcmp(buf, "ถึง") == 0) tokens[tok_count].type = TOK_TO;
                else tokens[tok_count].type = TOK_ID;
                
                strcpy(tokens[tok_count].text, buf);
                tok_count++;
            }
        }
    }
    
    tokens[tok_count].type = TOK_EOF;
    strcpy(tokens[tok_count].text, "");
    tok_count++;
}

Node *make_num_ld(long double v, int scale)
{
    Node *n = malloc(sizeof(Node));
    n->type = NODE_NUM;
    n->fvalue = v;
    n->scale = scale;
    n->left = NULL;
    n->right = NULL;
    n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_var(const char *name)
{
    Node *n = malloc(sizeof(Node));
    n->type = NODE_VAR;
    strcpy(n->varname, name);
    n->left = NULL;
    n->right = NULL;
    n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_binop(char op, Node *l, Node *r)
{
    Node *n = malloc(sizeof(Node));
    n->type = NODE_BINOP;
    n->op = op;
    n->left = l;
    n->right = r;
    n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_unary(char op, Node *operand)
{
    Node *n = malloc(sizeof(Node));
    n->type = NODE_UNARY;
    n->op = op;
    n->operand = operand;
    n->left = NULL;
    n->right = NULL;
    n->arg_count = 0;
    return n;
}

Node *make_func_call(const char *name)
{
    Node *n = malloc(sizeof(Node));
    n->type = NODE_FUNC_CALL;
    strcpy(n->varname, name);
    n->left = NULL;
    n->right = NULL;
    n->operand = NULL;
    n->arg_count = 0;
    return n;
}

Node *parse_expr();

Node *parse_factor()
{
    Token *t = peek();

    if (t->type == TOK_SQRT || t->type == TOK_ABS || 
        t->type == TOK_FLOOR || t->type == TOK_CEIL)
    {
        char op = t->type;
        next();
        
        if (peek()->type == TOK_LPAREN) next();
        
        Node *operand = parse_expr();
        
        if (peek()->type == TOK_RPAREN) next();
        
        return make_unary(op, operand);
    }

    if (t->type == TOK_LEN)
    {
        next();
        if (peek()->type == TOK_LPAREN) next();
        
        if (peek()->type == TOK_ID)
        {
            Token *arr_name = next();
            Node *n = malloc(sizeof(Node));
            n->type = NODE_UNARY;
            n->op = TOK_LEN;
            n->operand = make_var(arr_name->text);
            n->left = NULL;
            n->right = NULL;
            n->arg_count = 0;
            
            if (peek()->type == TOK_RPAREN) next();
            
            return n;
        }
        return make_num_ld(0.0L, 0);
    }

    if (t->type == TOK_MINUS)
    {
        next();
        return make_unary('-', parse_factor());
    }

    if (t->type == TOK_LPAREN)
    {
        next();
        Node *node = parse_expr();
        if (peek()->type == TOK_RPAREN) next();
        return node;
    }

    if (t->type == TOK_NUM)
    {
        next();
        Value val = parse_decimal_lexeme(t->text);
        return make_num_ld(val.v, val.scale);
    }

    if (t->type == TOK_ID)
    {
        char name[64];
        strcpy(name, t->text);
        next();

        if (peek()->type == TOK_LBRACKET)
        {
            Node *arr = make_var(name);
            arr->type = NODE_ARRAY_ACCESS;
            next();
            arr->left = parse_expr();
            
            if (peek()->type == TOK_RBRACKET) next();
            
            return arr;
        }

        if (peek()->type == TOK_LPAREN)
        {
            next();
            Node *func = make_func_call(name);
            
            while (peek()->type != TOK_RPAREN && peek()->type != TOK_EOF)
            {
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

Node *parse_power()
{
    Node *node = parse_factor();
    while (peek()->type == TOK_POWER)
    {
        next();
        node = make_binop('^', node, parse_factor());
    }
    return node;
}

Node *parse_term()
{
    Node *node = parse_power();
    while (peek()->type == TOK_MUL || peek()->type == TOK_DIV || peek()->type == TOK_MOD)
    {
        char op = peek()->text[0];
        next();
        node = make_binop(op, node, parse_power());
    }
    return node;
}

Node *parse_expr()
{
    Node *node = parse_term();
    while (peek()->type == TOK_PLUS || peek()->type == TOK_MINUS)
    {
        char op = peek()->text[0];
        next();
        node = make_binop(op, node, parse_term());
    }
    return node;
}

static Value bin_calc(char op, Value l, Value r)
{
    Value out;
    
    switch (op)
    {
        case '+':
            out.v = l.v + r.v;
            out.scale = max_int(l.scale, r.scale);
            break;
            
        case '-':
            out.v = l.v - r.v;
            out.scale = max_int(l.scale, r.scale);
            break;
            
        case '*':
            out.v = l.v * r.v;
            out.scale = max_int(l.scale, r.scale);
            break;
            
        case '/':
            if (r.v == 0.0L)
            {
                out.v = 0.0L;
                out.scale = 0;
            }
            else
            {
                out.v = l.v / r.v;
                out.scale = max_int(max_int(l.scale, r.scale), 2);
            }
            break;
            
        case '%':
            out.v = fmodl(l.v, r.v);
            out.scale = max_int(l.scale, r.scale);
            break;
            
        case '^':
            out.v = powl(l.v, r.v);
            out.scale = max_int(l.scale, r.scale);
            break;
            
        default:
            out.v = 0.0L;
            out.scale = 0;
            break;
    }
    
    return out;
}

void execute_commands(char **cmds, int count);

static Value eval_node(struct Node *n)
{
    if (n == NULL) return make_value(0.0L, 0);

    if (n->type == NODE_NUM)
    {
        return make_value(n->fvalue, n->scale);
    }

    if (n->type == NODE_VAR)
    {
        if (get_array(n->varname) != NULL)
        {
            return make_value(0.0L, 0);
        }

        if (find_variable_index(n->varname) == -1)
        {
            set_variable(n->varname, make_value(0.0L, 0));
        }
        return get_variable(n->varname);
    }

    if (n->type == NODE_ARRAY_ACCESS)
    {
        Array *arr = get_array(n->varname);
        if (!arr) return make_value(0.0L, 0);

        Value idx_val = eval_node(n->left);
        int idx = (int)idx_val.v;

        if (idx < 0 || idx >= arr->size) return make_value(0.0L, 0);
        return arr->data[idx];
    }

    if (n->type == NODE_FUNC_CALL)
    {
        Function *func = get_function(n->varname);
        if (!func) return make_value(0.0L, 0);

        int saved_has_return = g_has_return;
        Value saved_return_value = g_return_value;
        g_has_return = 0;

        current_scope_depth++;

        for (int i = 0; i < func->param_count && i < n->arg_count; i++)
        {
            Value arg_val = eval_node(n->args[i]);
            if (var_count < MAX_VARIABLES) {
                strcpy(variables[var_count].name, func->params[i]);
                variables[var_count].value = arg_val;
                variables[var_count].depth = current_scope_depth;
                var_count++;
            }
        }

        execute_commands(func->body, func->body_count);

        Value result = g_has_return ? g_return_value : make_value(0.0L, 0);

        clear_variables_by_depth(current_scope_depth);
        current_scope_depth--;

        g_has_return = saved_has_return;
        g_return_value = saved_return_value;

        return result;
    }

    if (n->type == NODE_UNARY)
    {
        if (n->op == TOK_LEN)
        {
            if (n->operand && n->operand->type == NODE_VAR)
            {
                Array *arr = get_array(n->operand->varname);
                if (arr) return make_value((long double)arr->size, 0);
            }
            return make_value(0.0L, 0);
        }

        Value operand = eval_node(n->operand);
        Value out;
        
        switch (n->op)
        {
            case '-':
                out.v = -operand.v;
                out.scale = operand.scale;
                break;
            case TOK_SQRT:
                out.v = sqrtl(operand.v);
                out.scale = max_int(operand.scale, 2);
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

    if (n->type == NODE_BINOP)
    {
        Value l = eval_node(n->left);
        Value r = eval_node(n->right);
        return bin_calc(n->op, l, r);
    }

    return make_value(0.0L, 0);
}

void add_command(const char *line)
{
    if (cmd_count < MAX_CMDS)
    {
        #ifdef _WIN32
            cmd_buffer[cmd_count] = _strdup(line);
        #else
            cmd_buffer[cmd_count] = strdup(line);
        #endif
        cmd_count++;
    }
}

int eval_condition_from_tokpos()
{
    if (peek()->type == TOK_NOT)
    {
        next();
        return !eval_condition_from_tokpos();
    }

    Node *left_expr = parse_expr();
    Value left_val = eval_node(left_expr);

    Token *cmp = peek();

    if (cmp->type == TOK_LT || cmp->type == TOK_GT || cmp->type == TOK_LE || cmp->type == TOK_GE ||
        cmp->type == TOK_EQEQ || cmp->type == TOK_NEQ)
    {
        TokenType cmpType = cmp->type;
        next();

        Node *right_expr = parse_expr();
        Value right_val = eval_node(right_expr);

        long double L = left_val.v;
        long double R = right_val.v;
        int result;

        switch (cmpType)
        {
            case TOK_LT: result = L < R; break;
            case TOK_GT: result = L > R; break;
            case TOK_LE: result = L <= R; break;
            case TOK_GE: result = L >= R; break;
            case TOK_EQEQ: result = fabsl(L - R) < 1e-10L; break;
            case TOK_NEQ: result = fabsl(L - R) >= 1e-10L; break;
            default: result = 0;
        }

        if (peek()->type == TOK_AND)
        {
            next();
            return result && eval_condition_from_tokpos();
        }
        
        if (peek()->type == TOK_OR)
        {
            next();
            return result || eval_condition_from_tokpos();
        }

        return result;
    }

    if (cmp->type == TOK_AND)
    {
        next();
        return (left_val.v != 0.0L) && eval_condition_from_tokpos();
    }
    
    if (cmp->type == TOK_OR)
    {
        next();
        return (left_val.v != 0.0L) || eval_condition_from_tokpos();
    }

    return left_val.v != 0.0L;
}

static Value parse_input_value()
{
    char buf[256];
    if (!fgets(buf, sizeof(buf), stdin)) return make_value(0.0L, 0);

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

void execute_commands(char **cmds, int count)
{
    for (int i = 0; i < count && !g_has_return && !g_break_flag; i++)
    {
        if (g_continue_flag)
        {
            g_continue_flag = 0;
            continue;
        }

        lex_line(cmds[i]);

        if (tok_count <= 1) continue;
        if (tokens[0].type == TOK_END) continue;

        if (tokens[0].type == TOK_FUNC)
        {
            tok_pos = 1;
            Token *func_name = next();

            char params[MAX_FUNC_PARAMS][64];
            int param_count = 0;

            if (peek()->type == TOK_LPAREN)
            {
                next();
                while (peek()->type != TOK_RPAREN && peek()->type != TOK_EOF)
                {
                    if (next()->type == TOK_ID)
                    {
                        strcpy(params[param_count++], tokens[tok_pos-1].text);
                    }
                    if (peek()->type == TOK_COMMA) next();
                }
                if (peek()->type == TOK_RPAREN) next();
            }

            char *body[MAX_CMDS];
            int body_count = 0;
            i++;
            int depth = 1;

            while (i < count)
            {
                lex_line(cmds[i]);
                
                if (tokens[0].type == TOK_FUNC || tokens[0].type == TOK_IF || 
                    tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR)
                {
                    depth++;
                }

                if (tokens[0].type == TOK_END)
                {
                    depth--;
                    if (depth == 0) break;
                }
                
                body[body_count++] = cmds[i];
                i++;
            }
            add_function(func_name->text, params, param_count, body, body_count);
            continue;
        }

        if (tokens[0].type == TOK_RETURN)
        {
            tok_pos = 1;
            if (peek()->type != TOK_EOF)
            {
                g_return_value = eval_node(parse_expr());
            }
            else
            {
                g_return_value = make_value(0.0L, 0);
            }
            g_has_return = 1;
            return;
        }

        if (tokens[0].type == TOK_BREAK)
        {
            g_break_flag = 1;
            return;
        }

        if (tokens[0].type == TOK_CONTINUE)
        {
            g_continue_flag = 1;
            continue;
        }

        if (tokens[0].type == TOK_WHILE)
        {
            int start_line = i;

            char **loop_body = malloc(sizeof(char*) * MAX_CMDS);
            int loop_body_count = 0;
            i++;
            int depth = 1;

            while (i < count)
            {
                lex_line(cmds[i]);
                
                if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                    tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC)
                {
                    depth++;
                }

                if (tokens[0].type == TOK_END)
                {
                    depth--;
                    if (depth == 0) break;
                }
                
                loop_body[loop_body_count++] = cmds[i];
                i++;
            }

            while (1)
            {
                lex_line(cmds[start_line]);
                tok_pos = 1;

                int cond = eval_condition_from_tokpos();

                if (!cond || g_break_flag)
                {
                    g_break_flag = 0;
                    break;
                }

                execute_commands(loop_body, loop_body_count);
                
                if (g_has_return) break;
            }
            
            free(loop_body);
            continue;
        }

        if (tokens[0].type == TOK_FOR)
        {
            tok_pos = 1;
            Token *loop_var = next();
            char loop_var_name[64];
            strcpy(loop_var_name, loop_var->text);

            if (peek()->type == TOK_ASSIGN) next();

            Value start_val = eval_node(parse_expr());

            if (peek()->type == TOK_TO) 
            {
                next();
            }
            else 
            {
                while (peek()->type != TOK_NUM && peek()->type != TOK_MINUS && 
                       peek()->type != TOK_LPAREN && peek()->type != TOK_EOF)
                {
                    next();
                }
            }

            Value end_val = eval_node(parse_expr());

            char **loop_body = malloc(sizeof(char*) * MAX_CMDS);
            int loop_body_count = 0;
            i++;
            int depth = 1;

            while (i < count)
            {
                lex_line(cmds[i]);
                
                if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                    tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC)
                {
                    depth++;
                }

                if (tokens[0].type == TOK_END)
                {
                    depth--;
                    if (depth == 0) break;
                }
                
                loop_body[loop_body_count++] = cmds[i];
                i++;
            }

            for (long double v = start_val.v; v <= end_val.v; v += 1.0L)
            {
                set_variable(loop_var_name, make_value(v, 0));

                if (g_break_flag)
                {
                    g_break_flag = 0;
                    break;
                }

                execute_commands(loop_body, loop_body_count);
                
                if (g_has_return) break;
            }
            
            free(loop_body);
            continue;
        }

        if (tokens[0].type == TOK_IF)
        {
            tok_pos = 1;
            int cond = eval_condition_from_tokpos();

            char **if_body = malloc(sizeof(char*) * MAX_CMDS);
            int if_cnt = 0;
            i++;
            int depth = 1;

            while (i < count)
            {
                lex_line(cmds[i]);
                
                if (depth == 1 && tokens[0].type == TOK_ELSE) break;

                if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                    tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC)
                {
                    depth++;
                }

                if (tokens[0].type == TOK_END)
                {
                    depth--;
                    if (depth == 0) break;
                }
                
                if_body[if_cnt++] = cmds[i];
                i++;
            }

            char **else_body = malloc(sizeof(char*) * MAX_CMDS);
            int else_cnt = 0;

            if (i < count && tokens[0].type == TOK_ELSE)
            {
                i++;
                depth = 1;
                while (i < count)
                {
                    lex_line(cmds[i]);
                    
                    if (tokens[0].type == TOK_WHILE || tokens[0].type == TOK_FOR || 
                        tokens[0].type == TOK_IF || tokens[0].type == TOK_FUNC)
                    {
                        depth++;
                    }

                    if (tokens[0].type == TOK_END)
                    {
                        depth--;
                        if (depth == 0) break;
                    }
                    
                    else_body[else_cnt++] = cmds[i];
                    i++;
                }
            }

            if (cond)
            {
                execute_commands(if_body, if_cnt);
            }
            else
            {
                execute_commands(else_body, else_cnt);
            }

            free(if_body);
            free(else_body);
            continue;
        }

        if (tokens[0].type == TOK_GIVE)
        {
            tok_pos = 1;

            if (peek()->type == TOK_ID)
            {
                char var_name[64];
                strcpy(var_name, next()->text);

                if (peek()->type == TOK_LBRACKET)
                {
                    next();
                    int idx = (int)eval_node(parse_expr()).v;
                    next();
                    next();
                    array_set(var_name, idx, eval_node(parse_expr()));
                }
                else if (peek()->type == TOK_ASSIGN)
                {
                    next();
                    set_variable(var_name, eval_node(parse_expr()));
                }
                else
                {
                    Token *op = next();
                    Node *expr = parse_expr();

                    if (find_variable_index(var_name) == -1)
                    {
                        set_variable(var_name, make_value(0.0L, 0));
                    }

                    Value cur = get_variable(var_name);
                    char o = (op->type==TOK_PLUSEQ)?'+':(op->type==TOK_MINUSEQ)?'-':(op->type==TOK_MULEQ)?'*':'/';
                    set_variable(var_name, bin_calc(o, cur, eval_node(expr)));
                }
            }
            continue;
        }

        if (tokens[0].type == TOK_PRINTLN || tokens[0].type == TOK_PRINT)
        {
            int newline = (tokens[0].type == TOK_PRINTLN);
            tok_pos = 1;

            if (peek()->type == TOK_STRING)
            {
                printf(newline ? "%s\n" : "%s", next()->text);
                if (!newline) fflush(stdout);
            }
            else if (peek()->type != TOK_EOF)
            {
                print_value(eval_node(parse_expr()), newline);
            }
            else if (newline)
            {
                printf("\n");
            }
            continue;
        }

        if (tokens[0].type == TOK_FIND)
        {
            tok_pos = 1;
            print_value(eval_node(parse_expr()), 1);
            continue;
        }

        if (tokens[0].type == TOK_INPUT)
        {
            tok_pos = 1;
            Token *var = next();
            printf("กรอกค่า %s: ", var->text);
            fflush(stdout);
            set_variable(var->text, parse_input_value());
            continue;
        }

        if (tokens[0].type == TOK_ARRAY)
        {
            tok_pos = 1;
            Token *name = next();

            if (peek()->type == TOK_LBRACKET) next();

            create_array(name->text, (int)eval_node(parse_expr()).v);
            continue;
        }

        if (tokens[0].type == TOK_PUSH)
        {
            tok_pos = 1;
            Token *n = next();
            array_push(n->text, eval_node(parse_expr()));
            continue;
        }

        if (tokens[0].type == TOK_POP)
        {
            tok_pos = 1;
            array_pop(next()->text);
            continue;
        }

        if (tokens[0].type == TOK_CALL)
        {
            tok_pos = 1;
            Token *fn = next();
            Node *call = make_func_call(fn->text);

            if (peek()->type == TOK_LPAREN)
            {
                next();
                while (peek()->type != TOK_RPAREN && peek()->type != TOK_EOF)
                {
                    call->args[call->arg_count++] = parse_expr();
                    if (peek()->type == TOK_COMMA) next();
                }
                if (peek()->type == TOK_RPAREN) next();
            }
            eval_node(call);
            continue;
        }

        if (tokens[0].type == TOK_PREC)
        {
            tok_pos = 1;
            g_out_dp = (int)eval_node(parse_expr()).v;
            g_fixed_dp = 1;
            continue;
        }
    }
}

void run_all_commands()
{
    execute_commands(cmd_buffer, cmd_count);
    cmd_count = 0;
}

void __Ark_Shell()
{
    char line[256];
    setlocale(LC_NUMERIC, "C");
    printf("Aeroki Shell Mode (type 'ออก' to exit)\n");

    while (1)
    {
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

void __Ark_Interpreted(FILE *src)
{
    char line[256];
    setlocale(LC_NUMERIC, "C");

    while (fgets(line, sizeof(line), src))
    {
        line[strcspn(line, "\r\n")] = '\0';
        ltrim(line);

        if (strlen(line) == 0 || line[0] == '#') continue;

        add_command(line);
    }
    run_all_commands();
}
