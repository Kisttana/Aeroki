#ifndef ARK_LLEXER_H
#define ARK_LLEXER_H


#define MAX_LEN 256
#ifdef __cplusplus 

extern "C"{
#endif

// === tools ===
#include <regex.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

// === Data structure ===
#include <glib-2.0/glib.h>

typedef enum {
    TOKEN_UNKNOWN = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,

    // Keywords
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_LET,
    TOKEN_WHILE,
    TOKEN_FOR,

    // Operators and punctuation
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_LT,
    TOKEN_LTE,
    TOKEN_GT,
    TOKEN_GTE,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,

    TOKEN_EOF
} ARKTokenType;

typedef struct __TOKEN{
	ARKTokenType _Type;
	char _Value[MAX_LEN];
}ARKToken;

typedef GArray ARKTokenList;

#define getTkidx(__src, __type , __idx) g_array_index(__src , __type , __idx);
#define init_TokenList(__size) g_array_sized_new(FALSE, FALSE, sizeof (ARKToken), __size)
#define TokenList_resize(__size) g_array_set_size(FALSE, __size)
typedef struct __LEXER{
	   	char * lexeme;
		size_t begin;
		size_t cursor;
}ARKLexer;
ARKTokenType determine_token_type(const char* lexeme);
ARKTokenList* Laxer(const char * _s);
ARKToken generate_token(const char * _s,size_t begin, int (*analyzer)(int));
ARKLexer init_lexer(char * _s);

#ifdef __cplusplus
}
#endif


#endif // ARK_LLEXER_H

