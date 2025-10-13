#ifndef __PRINT_TOKENTYPE__
#define __PRINT_TOKENTYPE__


static const char *TOKEN_TYPE[] = {
    "TOKEN_UNKNOWN",
    "TOKEN_IDENTIFIER",
    "TOKEN_NUMBER",
    "TOKEN_STRING",
    "TOKEN_TRUE",
    "TOKEN_FALSE",
    // Keywords
    "TOKEN_IF",
    "TOKEN_ELSE",
    "TOKEN_RETURN",
    "TOKEN_LET",
    "TOKEN_WHILE",
    "TOKEN_FOR",
    "TOKEN_DEF",
    "TOKEN_AND",
    "TOKEN_OR",
    // Operators and punctuation
    "TOKEN_PLUS",
    "TOKEN_MINUS",
    "TOKEN_MUL",
    "TOKEN_DIV",
    "TOKEN_POW",
    "TOKEN_MOD",
    "TOKEN_BANG",
    "TOKEN_EQUAL",
    "TOKEN_EQUAL_EQUAL",
    "TOKEN_NOT_EQUAL",
    "TOKEN_LT",
    "TOKEN_LTE",
    "TOKEN_GT",
    "TOKEN_GTE",
    "TOKEN_SEMICOLON",
    "TOKEN_COMMA",
    "TOKEN_DOT",
    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
    "TOKEN_LBRACE",
    "TOKEN_RBRACE",
    "TOKEN_SHAPE",
    "TOKEN_EOF",
};

#ifndef TYPEOF
#define TYPEOF
#define typeof(__TOKEN) TOKEN_TYPE[__TOKEN] 
#endif


#endif //__PRINT_TOKENTYPE__

