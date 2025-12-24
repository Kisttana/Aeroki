#include "Lexer.h"
#include "read_file.h"
#include "tokenizer.h"



ArkLexer *_Ark_ScanTokens(const char *file_to_scan)
{
     ArkLexer *newLexer =  new(ArkLexer);
     newLexer->token_list = vec_init(VEC_DEFLUAT_CAPACITY, ArkToken, 0);


     newLexer->file = fopen( file_to_scan,"r");
     if(newLexer->file == NULL){
          perror("can't open file"); 
          exit(EXIT_FAILURE);
     }
     newLexer->buffer = read_file(newLexer->file); 
     newLexer->buffer = newLexer->buffer;
     newLexer->cursor = newLexer->begin;

     size_t index = 0;
     
     uint32_t line = 1;
     char token_name[MAX_TOKEN_LEN + 1]= { '\0' };
     ArkToken token;
     char *letter;
     size_t cursor = 0;
     while ( ( letter = newLexer->cursor ) != NULL ) {
          
          memset(token_name, '\0', sizeof(token_name));

          if ( letter[0] == '\n') {
               ++line;
               ++newLexer->cursor;
               newLexer->begin = newLexer->cursor;
               continue;
          }
          
          if(  letter[0] == '\"'){
              _Ark_ReadString(token.lexeme, letter, &cursor); 
               continue;
          }

          else if(  ispunct(letter[0]) ){
               int type = _ArkToken_OneChar(letter[0]);

               token.punct[0] = letter[0]; 
               if(ispunct(letter[1])){
                    token.punct[1] = letter[1];

                    if( ispunct(letter[2]) ){
                         token.punct[2] = letter[2];
                         type = _ArkToken_ThreeChars(letter[0], letter[1], letter[3]);
                    }
                    else type = _ArkToken_TwoChars(letter[0], letter[1]);
               }
          
          }
     

     }
 

     return newLexer;
}
/**/
int main(){

}
