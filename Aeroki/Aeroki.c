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
}
