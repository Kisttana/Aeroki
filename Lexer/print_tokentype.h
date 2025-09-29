#ifndef __PRINT_TOKENTYPE__
#define __PRINT_TOKENTYPE__



const char * TOKEN_TYPE[] = {
	"TOKEN_UNKNOWN",
    "TOKEN_IDENTIFIER",
    "TOKEN_NUMBER",
    "TOKEN_STRING",

    // Keywords
    "TOKEN_IF",
    "TOKEN_ELSE",
    "TOKEN_RETURN",
    "TOKEN_LET",
    "TOKEN_WHILE",
	"TOKEN_FOR",

    // Operators and punctuation
    "TOKEN_PLUS",
    "TOKEN_MINUS",
    "TOKEN_STAR",
    "TOKEN_SLASH",
    "TOKEN_EQUAL",
    "TOKEN_EQUAL_EQUAL",
    "TOKEN_NOT_EQUAL",
    "TOKEN_LT",
    "TOKEN_LTE",
    "TOKEN_GT",
    "TOKEN_GTE",
    "TOKEN_SEMICOLON",
    "TOKEN_COMMA",
    "TOKEN_LPAREN",
    "TOKEN_RPAREN",
    "TOKEN_LBRACE",
    "TOKEN_RBRACE",

    "TOKEN_EOF"
 
};


#ifndef TYPEOF
#define TYPEOF
#define typeof(__TOKEN) (((__TOKEN) <= 27 && (__TOKEN) >= 0)?(TOKEN_TYPE[__TOKEN]):(TOKEN_TYPE[0])) 
#endif


#endif //__PRINT_TOKENTYPE__

