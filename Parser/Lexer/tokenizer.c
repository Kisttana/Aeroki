#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

int
_ArkToken_OneChar(int c1)
{
    switch (c1) {
    case '!': return EXCLAMATION;
    case '%': return PERCENT;
    case '&': return AMPER;
    case '(': return LPAR;
    case ')': return RPAR;
    case '*': return STAR;
    case '+': return PLUS;
    case ',': return COMMA;
    case '-': return MINUS;
    case '.': return DOT;
    case '/': return SLASH;
    case ':': return COLON;
    case ';': return SEMI;
    case '<': return LESS;
    case '=': return EQUAL;
    case '>': return GREATER;
    case '@': return AT;
    case '[': return LSQB;
    case ']': return RSQB;
    case '^': return CIRCUMFLEX;
    case '{': return LBRACE;
    case '|': return VBAR;
    case '}': return RBRACE;
    case '~': return TILDE;
    }
    return OP;
}

int
_ArkToken_TwoChars(int c1, int c2)
{
    switch (c1) {
    case '!':
        switch (c2) {
        case '=': return NOTEQUAL;
        }
        break;
    case '%':
        switch (c2) {
        case '=': return PERCENTEQUAL;
        }
        break;
    case '&':
        switch (c2) {
        case '=': return AMPEREQUAL;
        }
        break;
    case '*':
        switch (c2) {
        case '*': return DOUBLESTAR;
        case '=': return STAREQUAL;
        }
        break;
    case '+':
        switch (c2) {
        case '=': return PLUSEQUAL;
        }
        break;
    case '-':
        switch (c2) {
        case '=': return MINEQUAL;
        case '>': return RARROW;
        }
        break;
    case '/':
        switch (c2) {
        case '/': return DOUBLESLASH;
        case '=': return SLASHEQUAL;
        }
        break;
    case ':':
        switch (c2) {
        case '=': return COLONEQUAL;
        }
        break;
    case '<':
        switch (c2) {
        case '<': return LEFTSHIFT;
        case '=': return LESSEQUAL;
        case '>': return NOTEQUAL;
        }
        break;
    case '=':
        switch (c2) {
        case '=': return EQEQUAL;
        }
        break;
    case '>':
        switch (c2) {
        case '=': return GREATEREQUAL;
        case '>': return RIGHTSHIFT;
        }
        break;
    case '@':
        switch (c2) {
        case '=': return ATEQUAL;
        }
        break;
    case '^':
        switch (c2) {
        case '=': return CIRCUMFLEXEQUAL;
        }
        break;
    case '|':
        switch (c2) {
        case '=': return VBAREQUAL;
        }
        break;
    }
    return OP;
}

int
_ArkToken_ThreeChars(int c1, int c2, int c3)
{
    switch (c1) {
    case '*':
        switch (c2) {
        case '*':
            switch (c3) {
            case '=': return DOUBLESTAREQUAL;
            }
            break;
        }
        break;
    case '.':
        switch (c2) {
        case '.':
            switch (c3) {
            case '.': return ELLIPSIS;
            }
            break;
        }
        break;
    case '/':
        switch (c2) {
        case '/':
            switch (c3) {
            case '=': return DOUBLESLASHEQUAL;
            }
            break;
        }
        break;
    case '<':
        switch (c2) {
        case '<':
            switch (c3) {
            case '=': return LEFTSHIFTEQUAL;
            }
            break;
        }
        break;
    case '>':
        switch (c2) {
        case '>':
            switch (c3) {
            case '=': return RIGHTSHIFTEQUAL;
            }
            break;
        }
        break;
    }
    return OP;
}
void _Ark_ReadString(char *dest, char *src,size_t *cursor)
{
     // check if string is not found
     if ( src[0] == '\"' ) {
          char *endString = strchr(src + 1 , '\"');
          size_t len = endString - src + 1;
	  cursor+=len;
          strncpy(dest, src, len);
     }
}
void _Ark_ReadToken(
     char *dest,
     const char *src, 
     size_t *cursor, 
     int (*func)(int))
{
     size_t idx = 0;

     while (src[*cursor] != '\0' && func(src[*cursor]) ) {
          dest[idx++] = src[(*cursor)++];
          if(idx >= MAX_TOKEN_LEN + 1){
               fprintf(stderr, "Token too long\n");
               exit(EXIT_FAILURE);
          }
     }
}
