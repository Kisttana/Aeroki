#include "Aeroki.h"
#include "../sysinfo/sysinfo.h"
#include <unistd.h>
int main(int argc, char *argv[]){
     if(argc < 2) 
          ARKsuggestion();	


     printf("run :%s version : %s \n", argv[0], VERSION );
     printf("Compiling file : %s\n", argv[1]);	

     char * stringSrc_code = load_file(argv[1]);

     puts(stringSrc_code);


     ARKLexer _lex = init_lexer(stringSrc_code);
     printf("Lexer initialization succees.\n");

     vector *TokenList = get_tokens(&_lex);

     for(size_t i=0;i<TokenList->size ;++i){

          ARKToken ptoken = vec_at(TokenList,ARKToken , i);
          printf("token type : %s  , value : %s\n", typeof(ptoken._Type), ptoken._Value);
     }
     
     
     
     return 0;
}
