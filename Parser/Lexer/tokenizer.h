#ifndef ARK_TOKENIZER_H
#define ARK_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <string.h>
#include <ctype.h>
#define new(type) ((type*)malloc(sizeof(type)))
#define MAX_TOKEN_LEN 128
#define SPACE ' '

typedef int ArkTokenType;
#define ENDMARKER       0
#define NAME            1
#define NUMBER          2
#define STRING          3
#define NEWLINE         4
#define INDENT          5
#define DEDENT          6
#define LPAR            7
#define RPAR            8
#define LSQB            9
#define RSQB            10
#define COLON           11
#define COMMA           12
#define SEMI            13
#define PLUS            14
#define MINUS           15
#define STAR            16
#define SLASH           17
#define VBAR            18
#define AMPER           19
#define LESS            20
#define GREATER         21
#define EQUAL           22
#define DOT             23
#define PERCENT         24
#define LBRACE          25
#define RBRACE          26
#define EQEQUAL         27
#define NOTEQUAL        28
#define LESSEQUAL       29
#define GREATEREQUAL    30
#define TILDE           31
#define CIRCUMFLEX      32
#define LEFTSHIFT       33
#define RIGHTSHIFT      34
#define DOUBLESTAR      35
#define PLUSEQUAL       36
#define MINEQUAL        37
#define STAREQUAL       38
#define SLASHEQUAL      39
#define PERCENTEQUAL    40
#define AMPEREQUAL      41
#define VBAREQUAL       42
#define CIRCUMFLEXEQUAL 43
#define LEFTSHIFTEQUAL  44
#define RIGHTSHIFTEQUAL 45
#define DOUBLESTAREQUAL 46
#define DOUBLESLASH     47
#define DOUBLESLASHEQUAL 48
#define AT              49
#define ATEQUAL         50
#define RARROW          51
#define ELLIPSIS        52
#define COLONEQUAL      53
#define EXCLAMATION     54
#define OP              55
#define TYPE_IGNORE     56
#define TYPE_COMMENT    57
#define SOFT_KEYWORD    58
#define FSTRING_START   59
#define FSTRING_MIDDLE  60
#define FSTRING_END     61
#define TSTRING_START   62
#define TSTRING_MIDDLE  63
#define TSTRING_END     64
#define COMMENT         65
#define NL              66
#define ERRORTOKEN      67

/* Token names */


/* 
 * @brief Determine token type 
 * @return The token type corresponding to the charater
*/

int _ArkToken_OneChar(int c1);
int _ArkToken_TwoChars(int c1,int c2);
int _ArkToken_ThreeChars(int c1,int c2, int c3);


/* Function : isIdenifier
 * @brief check the charater is "alphabet , 0-9 , '_'(under score)".
 * 
 * @param int c A character to check .
 * @return 1 if it's alphanumeric and '_' (under score).
 *         0 if it's not.
 */
static inline int _Ark_isIden(int c) {
     return (isalnum(c) || c == '_');
}
/* Function : _Ark_read_string
 * @brief Handle reading string when encouter (" quote ) 
 * For example -> "This is string"
 *
 * @param char *dest Destination to put string into
 * @param char *src Source string to read 
 * @param size_t *cursor A pointer to src cursor
*/
void _Ark_ReadString(char *dest, char *src, size_t *cursor);


/* Function : _Ark_ReadToken
 * @brief Handle reading Token with condition.
 * 
 * @param char *dest Destination to put string into.
 * @param char *src Starting pointer of raw string. 
 * @param size_t *cursor A pointer to src cursor. 
 * @param int (*func)(int) A callback function returning of condition status 
 *            which use for reading the token 
*/
 void _Ark_ReadToken( char *dest, const char *src, size_t *len ,int (*func)(int) );



#ifdef __cplusplus
}
#endif

#endif // ARK_TOKENIZER_H
