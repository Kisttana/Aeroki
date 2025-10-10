#include "Aeroki.h"
#include "../sysinfo/sysinfo.h"
int main(int argc, char *argv[]){
     if(argc < 2) 
          ARKsuggestion();	


     printf("run :%s\n", argv[0], VERSION );
     printf("Compiling file : %s\n", argv[1]);	

     char * stringSrc_code = load_file(argv[1]);

     puts(stringSrc_code);


     ARKLexer _lex = init_lexer(stringSrc_code);
     printf("Lexer initialization succees.\n");

     ARKTokenList *TokenList = scanLexer(&_lex);

     for(size_t i=0;i<TokenList->len ;++i){

          ARKToken ptoken = g_array_index(TokenList,ARKToken , i);
          printf("token type : %s  , value : %s\n", typeof(ptoken._Type), ptoken._Value);
     }
     
     initRoutine(TokenList);
     AST *ast = ParseTopLevelExpr(); // or ParseDefinition/Parse based on the input
     printAST(ast, 0);
     freeAST(ast);
     g_array_free(TokenList,true);
     return 0;
}
