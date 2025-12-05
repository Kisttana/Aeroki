#ifndef ARK_LEXER_H
#define ARK_LEXER_H


#define MAX_LEN 256
#ifdef __cplusplus 

extern "C"{
#endif

// === tools ===
#include "../libs/Aegis/include/aegis/aegis_vector.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


// === Data structure ===

typedef enum {
     TOKEN_UNKNOWN = 0,
     TOKEN_IDENTIFIER,
     TOKEN_NUMBER,
     TOKEN_STRING,
     TOKEN_TRUE,
     TOKEN_FALSE,
     // Keywords
     TOKEN_IF,
     TOKEN_ELSE,
     TOKEN_RETURN,
     TOKEN_LET,
     TOKEN_WHILE,
     TOKEN_FOR,
     TOKEN_DEF,
     TOKEN_AND,
     TOKEN_OR,
     // Operators and punctuation
     TOKEN_PLUS,
     TOKEN_MINUS,
     TOKEN_MUL,
     TOKEN_DIV,
     TOKEN_POW,
     TOKEN_MOD,

     TOKEN_BANG,
     TOKEN_EQUAL,
     TOKEN_EQUAL_EQUAL,
     TOKEN_NOT_EQUAL,
     TOKEN_LT,
     TOKEN_LTE,
     TOKEN_GT,
     TOKEN_GTE,
     TOKEN_SEMICOLON,
     TOKEN_COMMA,
     TOKEN_DOT,
     TOKEN_LPAREN,
     TOKEN_RPAREN,

     TOKEN_LBRACE,
     TOKEN_RBRACE,
     TOKEN_SHAPE,
     

     TOKEN_EOF,
     TOKEN_NON_OF_TOKEN

} ARKTokenType;

typedef struct __TOKEN{
     ARKTokenType _Type;
     char _Value[MAX_LEN];
}ARKToken;



typedef struct {
     char * lexeme;
     size_t begin;
     size_t cursor;
}ARKLexer;

ARKTokenType determine_token_type(const char* lexeme);
vector* get_tokens(ARKLexer *lex);
ARKToken generate_token(const char * _s,size_t *cursor, int (*_Classifier)(int));
ARKLexer init_lexer(char * _s);

#ifdef __cplusplus
}
#endif


#endif // ARK_LLEXER_H

