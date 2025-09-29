#include "Lexer.h"

ARKTokenType determine_token_type(const char* lexeme){
    // Keywords
    if (strcmp(lexeme, "if") == 0) return TOKEN_IF;
    if (strcmp(lexeme, "else") == 0) return TOKEN_ELSE;
    if (strcmp(lexeme, "return") == 0) return TOKEN_RETURN;
    if (strcmp(lexeme, "let") == 0) return TOKEN_LET;
    if (strcmp(lexeme, "while") == 0) return TOKEN_WHILE;
    if (strcmp(lexeme, "for") == 0) return TOKEN_FOR;

    // Operators and punctuation
    if (strcmp(lexeme, "+") == 0) return TOKEN_PLUS;
    if (strcmp(lexeme, "-") == 0) return TOKEN_MINUS;
    if (strcmp(lexeme, "*") == 0) return TOKEN_STAR;
    if (strcmp(lexeme, "/") == 0) return TOKEN_SLASH;
    if (strcmp(lexeme, "=") == 0) return TOKEN_EQUAL;
    if (strcmp(lexeme, "==") == 0) return TOKEN_EQUAL_EQUAL;
    if (strcmp(lexeme, "!=") == 0) return TOKEN_NOT_EQUAL;
    if (strcmp(lexeme, "<") == 0) return TOKEN_LT;
    if (strcmp(lexeme, "<=") == 0) return TOKEN_LTE;
    if (strcmp(lexeme, ">") == 0) return TOKEN_GT;
    if (strcmp(lexeme, ">=") == 0) return TOKEN_GTE;
    if (strcmp(lexeme, ";") == 0) return TOKEN_SEMICOLON;
    if (strcmp(lexeme, ",") == 0) return TOKEN_COMMA;
    if (strcmp(lexeme, "(") == 0) return TOKEN_LPAREN;
    if (strcmp(lexeme, ")") == 0) return TOKEN_RPAREN;
    if (strcmp(lexeme, "{") == 0) return TOKEN_LBRACE;
    if (strcmp(lexeme, "}") == 0) return TOKEN_RBRACE;

    // Default to identifier or number
    return TOKEN_IDENTIFIER;
}
int isToken(int ch){
	return (int)(isalpha(ch) || ch == '_');
}

ARKToken generate_token(const char * _s,size_t begin, int (*analyzer)(int)){
		size_t idx = 0;
		ARKToken token;
		while (_s[begin] != '\0' && analyzer(_s[begin])) {
    		token._Value[idx++] = _s[begin++];
    		if (idx >= MAX_LEN - 1) {
        		fprintf(stderr, "Token too long\n");
        		exit(1);
    		}
		}
		token._Value[idx] = '\0';
		token._type = determine_token_type(token._Value);
		return token;
}

ARKTokenList* scanLexer(ARKLexer *lex) {
    ARKTokenList *ret = init_TokenList(64);
    char current;

    while ((current = lex->lexeme[lex->cursor]) != '\0') {
        if (isspace(current)) {
            lex->cursor++;
            lex->begin = lex->cursor;
            continue;
        }

        ARKToken token;

        if (isdigit(current)) {
            token = generate_token(lex->lexeme, lex->cursor, isdigit);
        } else if (isalpha(current) || current == '_') {
            token = generate_token(lex->lexeme, lex->cursor, isToken);
        } else if (ispunct(current)) {
            token = generate_token(lex->lexeme, lex->cursor, ispunct);
        } else {
            fprintf(stderr, "Unrecognized character: '%c'\n", current);
            lex->cursor++;
            continue;
        }

        g_array_append_val(ret, token);
        lex->cursor += strlen(token._Value);
        lex->begin = lex->cursor;
    }

    return ret;
}


