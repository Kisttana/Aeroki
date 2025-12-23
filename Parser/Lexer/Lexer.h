#ifndef ARK_LEXER_H
#define ARK_LEXER_H


// ======= Maximax length of lexeme ======

#ifdef __cplusplus 
extern "C"{
#endif

// === tools ===

#define _GNU_SOURCE
#include "../../libs/aegis/include/aegis.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "tokennizer.h"

/**
 * @struct Ark_lexer
 * @brief Define Ark_lexer as struct for handling lexeme's infomation.
 *
 * @var Ark_tokentype type Type of lexeme.
 * @var uint32_t line  Holding which is in the line for error handling.
 * @var ag_string lexeme => value string of lexeme.
 */
typedef struct _lexeme {
     Ark_tokentype  type;
     uint32_t       line;  
     ag_string      name;
}Arklexeme;

typedef struct _lexer {
     vector *Lexeme_list; /// dynamic array of Arklexeme
     FILE   *file; /// file to scan lexemes
}ArkLexer;

/**
 * @brief Create The List of lexemes
 *
 * @param file_to_scan Path to source code file to scan.
 * @return The vector (List) of lexemes
 * Note : type vector is a implementation of dynamic array from 
 * Aegis-Template-Library
 */
ArkLexer *_Ark_scan_lexemes(const char *file_to_scan);

#ifdef __cplusplus
}
#endif


#endif // ARK_LEXER_H

