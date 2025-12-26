/*
     author: Piyaphat Jaiboon
     contact: 
            email: fiw.contact.work@gmail.com
            github: https://github.com/Piyaph4t 
     date: June 10, 2024
     description: Implementation of the Lexer for tokenizing source code.
*/

#ifndef ARK_LEXER_H
#define ARK_LEXER_H

/**
 * @file Lexer.h
 * @brief Public API for the Ark lexer — tokenize Ark source files.
 *
 * This header declares the ArkLexer runtime state and the primary entry point
 * for scanning Ark source files into a vector of tokens (ArkToken).
 *
 * Key ownership and lifetime rules
 * --------------------------------
 * - The pointer returned by _Ark_ScanTokens() is heap-allocated and owned by
 *   the caller. The caller is responsible for freeing it and its internals.
 * - `ArkLexer->buffer` points to a NUL-terminated heap buffer that contains
 *   the full file contents; it is owned by the ArkLexer and must be freed
 *   when the ArkLexer is freed.
 * - `ArkToken.lexeme` is a fixed-size in-struct array and does not require
 *   a separate free.
 * - `ArkToken.string_literal` is heap-allocated for STRING tokens. The
 *   caller must free each `string_literal` (or call a helper cleanup that
 *   frees them) before freeing the ArkLexer.
 *
 * Errors and return values
 * -------------------------
 * - _Ark_ScanTokens() returns NULL on failure (allocation or file I/O failure).
 * - On success the returned ArkLexer contains a token_list vector with an
 *   EOF token appended at the end.
 *
 * Thread-safety
 * --------------
 * This API is NOT thread-safe. Concurrent access to the same ArkLexer
 * instance must be synchronized by the caller.
 *
 * Example usage
 * --------------
 * @code
 * ArkLexer *lexer = _Ark_ScanTokens("script.ark");
 * if (lexer) {
 *     // iterate tokens
 *     for (size_t i = 0; i < lexer->token_list->size; ++i) {
 *         ArkToken t = vec_at(lexer->token_list, ArkToken, i);
 *         // handle token
 *         if (t.type == STRING && t.string_literal) free(t.string_literal);
 *     }
 *     vec_clean(lexer->token_list);
 *     free(lexer->token_list);
 *     free(lexer->buffer);
 *     free(lexer->file);
 *     free(lexer);
 * }
 * @endcode
 *
 * @author Piyaphat Jaiboon
 * @date   June 10, 2024
 */

#ifdef __cplusplus 
extern "C"{
#endif

// === tools ===

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "../../libs/aegis/include/aegis/aegis_vector.h" //< include Aegis vector for dynamic array 
#include "tokenizer.h" //< include tokenizer for token definitions


/**
 * @struct ArkLexer
 * @brief Runtime state for a single lexer scanning operation.
 *
 * The ArkLexer object holds all runtime state required while scanning one
 * source file. The vector `token_list` stores copied ArkToken entries (value
 * semantics) — if a token contains a pointer (like string_literal) that pointer
 * is copied and still needs explicit freeing.
 *
 * Fields
 * - buffer: owned NUL-terminated file contents (malloc'd)
 * - begin: pointer to start of the current lexeme within buffer
 * - cursor: current scan position (pointer into buffer)
 * - file: strdup'd path to the scanned file (owned by ArkLexer)
 * - token_list: dynamic vector (Aegis) storing ArkToken values
 *
 * Ownership/usage
 * - The caller must free the ArkLexer and its internals as described in the
 *   file-level documentation. Consider providing a helper like
 *   `_Ark_FreeLexer(ArkLexer *lexer)` to centralize cleanup.
 */
typedef struct _lexer {
     char *buffer; //< Buffer for storing the raw string of file (owned).
     char *begin;  //< Start position of current lexeme (pointer into buffer).
     char *cursor; //< Current scan position (pointer into buffer).
     char *file;   //< Path to scanned file (strdup'ed, owned).
     vector *token_list; /// dynamic array of ArkToken (each element is a copy).
} ArkLexer;

/**
 * @brief Scan the given file and produce a populated ArkLexer.
 *
 * This function reads the file located at @p file_to_scan into an internal
 * buffer, tokenizes the contents, and appends tokens into the returned
 * ArkLexer->token_list vector. The vector elements are copies of ArkToken.
 *
 * @param[in] file_to_scan Path to the source file to tokenize. The function
 *                         will call strdup() on this string to store it in
 *                         the returned ArkLexer (caller still owns the original argument).
 * @return A pointer to a newly allocated ArkLexer on success, or NULL on
 *         failure (allocation or file I/O error). The caller is responsible
 *         for freeing the returned object and its internals (see file docs).
 */
ArkLexer *_Ark_ScanTokens(const char *file_to_scan);

#ifdef __cplusplus
}
#endif


#endif // ARK_LEXER_H

