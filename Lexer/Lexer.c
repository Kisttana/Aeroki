#include "Lexer.h"

int isIdentifier(int _c){
		return (int) (isalnum(_c) || _c == '_');
}

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

ARKToken generate_token(const char * _s,size_t *cursor, int (*_Classifier)(int)){
		if(_Classifier == NULL) {
				fprintf(stderr,"Could't reconize Classicfier\n");
				exit(1);
		}
		size_t idx = 0;
		ARKToken token;
		while (_s[*cursor] != '\0' && _Classifier(_s[*cursor])) {
    		token._Value[idx++] = _s[(*cursor)++];
    		if (idx >= MAX_LEN - 1) {
        		fprintf(stderr, "Token too long\n");
        		exit(1);
    		}
		}
		token._Value[idx] = '\0';
		token._Type = determine_token_type(token._Value);
		return token;
}

ARKTokenList* scanLexer(ARKLexer *lex) {
	printf("Lexeme : \n%s\n", lex->lexeme);
    ARKTokenList *ret = init_TokenList(1000);
    char current;

    ARKToken token;
    while ((current = lex->lexeme[lex->cursor]) != '\0') {
        if (isspace(current) || current == '\n') {
            lex->cursor++;
            lex->begin = lex->cursor;
            continue;
        }

		int (*Classifier)(int) = NULL;	
        if (isdigit(current))  Classifier = isdigit;
        else if (isIdentifier(current)) Classifier = isIdentifier;
        else if (ispunct(current)) Classifier = ispunct;
        else {
            fprintf(stderr, "Unrecognized character: '%c'\n", current);
            lex->cursor++;
            continue;
        }
			
        token = generate_token(lex->lexeme, &lex->cursor, Classifier);
        g_array_append_val(ret, token);
        lex->begin = lex->cursor;
    }

    return ret;
}

ARKLexer init_lexer(char * _s){
	return (ARKLexer){_s,0,0};
}
