#include "Lexer.h"

static int isIdentifier(int _c){
     return (int) (isalnum(_c) || _c == '_');
}

ARKTokenType determine_token_type(const char* lexeme){
     // Keywords
     if (strcmp(lexeme, "if") == 0) return TOKEN_IF;
     if (strcmp(lexeme, "else") == 0) return TOKEN_ELSE;
     if (    strcmp(lexeme, "and") == 0
          || strcmp(lexeme, "&&")  == 0) return TOKEN_AND; 
     if (    strcmp(lexeme, "or") == 0
          || strcmp(lexeme, "||")  == 0) return TOKEN_OR; 
     if (strcmp(lexeme, "return") == 0) return TOKEN_RETURN;
     if (strcmp(lexeme, "let") == 0) return TOKEN_LET;
     if (strcmp(lexeme, "while") == 0) return TOKEN_WHILE;
     if (strcmp(lexeme, "for") == 0) return TOKEN_FOR;

     // Operators and punctuation
     if (strcmp(lexeme, "+") == 0) return TOKEN_PLUS;
     if (strcmp(lexeme, "-") == 0) return TOKEN_MINUS;
     if (strcmp(lexeme, "*") == 0) return TOKEN_MUL;
     if (strcmp(lexeme, "**") == 0) return TOKEN_POW;
     if (strcmp(lexeme, "/") == 0) return TOKEN_DIV;
     if (strcmp(lexeme, "=") == 0) return TOKEN_EQUAL;
     if (strcmp(lexeme, "==") == 0) return TOKEN_EQUAL_EQUAL;
     if (strcmp(lexeme, "!=") == 0) return TOKEN_NOT_EQUAL;
     if (strcmp(lexeme, "<") == 0) return TOKEN_LT;
     if (strcmp(lexeme, "<=") == 0) return TOKEN_LTE;
     if (strcmp(lexeme, ">") == 0) return TOKEN_GT;
     if (strcmp(lexeme, ">=") == 0) return TOKEN_GTE;
     if (strcmp(lexeme, ";") == 0) return TOKEN_SEMICOLON;
     if (strcmp(lexeme, ",") == 0) return TOKEN_COMMA;
     if (strcmp(lexeme, ".") == 0) return TOKEN_DOT;
     if (strcmp(lexeme, "(") == 0) return TOKEN_LPAREN;
     if (strcmp(lexeme, ")") == 0) return TOKEN_RPAREN;
     if (strcmp(lexeme, "{") == 0) return TOKEN_LBRACE;
     if (strcmp(lexeme, "}") == 0) return TOKEN_RBRACE;
     
     size_t idx = 0 ;
     int isNumToken = 1;
     ARKTokenType RETURN_TOKEN = TOKEN_IDENTIFIER;
     while(lexeme[idx] != '\0'){
          if( !isdigit(lexeme[idx++]) )
               isNumToken = 0;  
     }
     if(isNumToken) 
          RETURN_TOKEN = TOKEN_NUMBER;
     // Default to identifier or number
     return RETURN_TOKEN;
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

vector* get_tokens(ARKLexer *lex) {
     printf("Lexeme : \n%s\n", lex->lexeme);
     vector *tokenlist = vec_init(0, ARKToken, 1);

     char current;

     ARKToken token;
     while ((current = lex->lexeme[lex->cursor]) != '\0') {
          
          // reset token to default
          
          memset(token._Value, '\0', sizeof(token._Value));
          
          if(current == '#'){
               char *search_begin = lex->lexeme + lex->cursor; 
               char * find_endl = 
                         strchr(search_begin,'\n');

               if(!find_endl) {
                    lex->cursor = sizeof(lex->lexeme)-1;
                    fprintf(stderr,"not found \n");
                    continue;
               }
               printf("found # comment the line \n");
               
               
     
               // update position to the end of line
               lex->cursor = find_endl - lex->lexeme + 1 ;
               lex->begin = lex->cursor;

               continue;
          }
               

          if (isspace(current) || current == '\n') {
               lex->cursor++;
               lex->begin = lex->cursor;
               continue;
          }

          int (*Classifier)(int) = NULL;	
          if (isdigit(current))  Classifier = isdigit;
          else if (isIdentifier(current)) Classifier = isIdentifier;

          else if(current == '\"'){ // check if it may be string
               size_t idx = 0;
               while(lex->lexeme[lex->cursor] != '\0'){

                    token._Value[idx++] = lex->lexeme[lex->cursor++];
                    printf("copy : %c\n" , token._Value[idx-1]);
                    if(lex->lexeme[lex->cursor] == '\"'){
                         token._Value[idx] = '\"';
                         token._Value[++idx] = '\0';
                         break;
                    }
               }
               lex->cursor++;
               token._Type =  (token._Value[0] == '\"') 
                    && (token._Value[idx-1] == '\"') ? 
                    TOKEN_STRING : TOKEN_UNKNOWN;


          }
          else if (ispunct(current) 
               || current == '+'
               || current == '-'
               || current == '/'
               || current == '*'
               || current == '.'
          ){
               
               if(     current == '/'  
                    && lex->lexeme[ lex->cursor + 1 ] == '*'
                 ){  
                    // check if it's multi-lines comment

                    char *search_begin = lex->lexeme + lex->cursor ;
                    char *find_end_comment =
                              strstr(search_begin, "*/");
                    if(!find_end_comment){
                         free(lex->lexeme);
                         fprintf(stderr, "SyntaxError : Unterminated Comment \n");
                         exit(EXIT_FAILURE);
                    }

                    lex->cursor = find_end_comment - lex->lexeme + 2;
                    lex->begin = lex->cursor; 
                    continue;
               }

               token._Value[0] = current;
               token._Value[1] = '\0' ;

               switch(lex->lexeme[lex->cursor+1]){
                    case '*':
                         token._Value[1] = '*';
                         lex->cursor++;
                         break;
                    case '/':
                         token._Value[1] = '/';
                         lex->cursor++;
                         break;
                    case '|':
                         token._Value[1] = '|';
                         lex->cursor++;
                         break;
                    case '&':
                         token._Value[1] = '&';
                         lex->cursor++;
                         break;
                    default :
                         break;
               }
               token._Value[2] = '\0';
               token._Type = determine_token_type(token._Value);
               lex->cursor++;
          }
          else {
               fprintf(stderr, "Unrecognized character: '%c'\n", current);
               lex->cursor++;
               continue;
          }


          if(Classifier != NULL)
               token = generate_token(lex->lexeme, &lex->cursor, Classifier);
          vec_push_back(tokenlist, ARKToken, token);
          lex->begin = lex->cursor;
     } 
     
     ARKToken eof =  {._Type=TOKEN_EOF,._Value="<EOF>"};
     vec_push_back(tokenlist, ARKToken, eof);
     return tokenlist;
}

ARKLexer init_lexer(char * _s){
     return (ARKLexer){_s,0,0};
}
