#include "tokenizer.h"

/* Token names - immutable lookup table */
extern const char* const ARK_TOKEN_NAMES[] = {
    "ENDMARKER",         // 0
    "NAME",              // 1
    "NUMBER",            // 2
    "STRING",            // 3
    "NEWLINE",           // 4
    "INDENT",            // 5
    "DEDENT",            // 6
    "LPAR",              // 7
    "RPAR",              // 8
    "LSQB",              // 9
    "RSQB",              // 10
    "COLON",             // 11
    "COMMA",             // 12
    "SEMI",              // 13
    "PLUS",              // 14
    "MINUS",             // 15
    "STAR",              // 16
    "SLASH",             // 17
    "VBAR",              // 18
    "AMPER",             // 19
    "LESS",              // 20
    "GREATER",           // 21
    "EQUAL",             // 22
    "DOT",               // 23
    "PERCENT",           // 24
    "LBRACE",            // 25
    "RBRACE",            // 26
    "EQEQUAL",           // 27
    "NOTEQUAL",          // 28
    "LESSEQUAL",         // 29
    "GREATEREQUAL",      // 30
    "TILDE",             // 31
    "CIRCUMFLEX",        // 32
    "LEFTSHIFT",         // 33
    "RIGHTSHIFT",        // 34
    "DOUBLESTAR",        // 35
    "PLUSEQUAL",         // 36
    "MINEQUAL",          // 37
    "STAREQUAL",         // 38
    "SLASHEQUAL",        // 39
    "PERCENTEQUAL",      // 40
    "AMPEREQUAL",        // 41
    "VBAREQUAL",         // 42
    "CIRCUMFLEXEQUAL",   // 43
    "LEFTSHIFTEQUAL",    // 44
    "RIGHTSHIFTEQUAL",   // 45
    "DOUBLESTAREQUAL",   // 46
    "DOUBLESLASH",       // 47
    "DOUBLESLASHEQUAL",  // 48
    "AT",                // 49
    "ATEQUAL",           // 50
    "RARROW",            // 51
    "ELLIPSIS",          // 52
    "COLONEQUAL",        // 53
    "EXCLAMATION",       // 54
    "OP",                // 55
    "TYPE_IGNORE",       // 56
    "TYPE_COMMENT",      // 57
    "SOFT_KEYWORD",      // 58
    "FSTRING_START",     // 59
    "FSTRING_MIDDLE",    // 60
    "FSTRING_END",       // 61
    "TSTRING_START",     // 62
    "TSTRING_MIDDLE",    // 63
    "TSTRING_END",       // 64
    "COMMENT",           // 65
    "NL",                // 66
    "ERRORTOKEN",        // 67
    "UNKNOWN_TOKEN",     // 68
    "EOF_TOKEN"          // 69
};

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
    return UNKNOWN_TOKEN;
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
    return UNKNOWN_TOKEN;
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
    return UNKNOWN_TOKEN;
}



int _Ark_isIden(int c){
     return isalpha(c) || c == '_' ;
}

int _Ark_isNumber(int c){
     return (isdigit(c) || c == '.' || c == '-' || c == 'e');
}
void
_Ark_ReadString(
     char **cursor, 
     ArkToken *token,
     ArkTokenType *type)
{

    char *start = *cursor + 1;
    char *endStringPtr = strchr( start, '\"') ;

     if(endStringPtr == NULL) {
          *type = ERRORTOKEN;
          return;
     }

     size_t LenString = (size_t)(endStringPtr - start);
     
     token->string_literal = (char*)malloc(LenString + 1);
     if(token->string_literal == NULL){
        *type = ERRORTOKEN;
     }
     memcpy(token->string_literal, start, LenString);
     token->string_literal[LenString] = '\0';

     *cursor = ++endStringPtr;
     *type = STRING;
}

void 
_Ark_ReadToken(
     char *dest,
     char **cursor, 
     int (*func)(int))
{
     size_t count = 0; 
     while (**cursor != '\0' && func((unsigned char)**cursor) ) {
          if (count + 1 < MAX_TOKEN_LEN) { 
               *(dest++) = *((*cursor)++);
               
          } else {
#ifdef DEBUG_LEN_TOKEN
               fprintf(stderr, "Token too long\n");
               exit(EXIT_FAILURE);
#else
               break;
#endif
          }
          
          ++count;
     }
}


void _Ark_ReadPunct(
     char **cursor, 
     ArkToken *token,
     ArkTokenType *type)
{
     unsigned char c1, c2, c3;
     c1 = (*cursor)[0];
     c2 = (*cursor)[1];
     c3 = (*cursor)[2];

    ArkTokenType ttype = _ArkToken_ThreeChars(c1, c2, c3);
    if (ttype != UNKNOWN_TOKEN) {
        token->lexeme[0] = c1;
        token->lexeme[1] = c2;
        token->lexeme[2] = c3;
        token->lexeme[3] = '\0';
        *type = ttype;
        (*cursor) += 3;
        return; 
    }
    ttype = _ArkToken_TwoChars(c1, c2);
    if (ttype != UNKNOWN_TOKEN) {
        token->lexeme[0] = c1;
        token->lexeme[1] = c2;
        token->lexeme[2] = '\0';
        *type = ttype;
        (*cursor) += 2;
        return;
    }
    ttype = _ArkToken_OneChar(c1);

    token->lexeme[0] = c1;
    token->lexeme[1] = '\0';
    *type = ttype;
    (*cursor) += 1;

    
}