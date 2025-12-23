#include "Lexer.h"
#include "read_file.h"

#define new(type) ((type*)mlloc(sizeof(type)))

ArkLexer *_Ark_scan_lexemes(const char *file_to_scan)
{
     ArkLexer *newLexer =  new(ArkLexer);

     newLexer->file = fopen( file_to_scan,"r");
     if(newLexer->file == NULL){
          perror("can't open file"); 
          exit(EXIT_FAILURE);
     }

}


