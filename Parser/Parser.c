#include "Parser.h" // AST , parse()

/*
    This make a managing Rountine to handle
    Token after generate from Lexer.
*/
static ARKToken CurTok;
static ARKTokenList *arrToken = NULL;

#define initRoutine(LIST_OF_TOKEN) arrToken = LIST_OF_TOKEN

static int getTokenPrec(const char * op){
    char *precedence[] = {"-" , "+", "*" , "/" , "**"};
    for(int i=0;i<5;++i)
    if(!strcmp(op,precedence[i])) 
        return i;
    return -1;
}


static void getNextToken(){
    static size_t CurIdxToken = 0;
    if(!arrToken){
        logError("arrToken haven't been initialized");
        exit(-1);
    }
    // Check Index out of range.
    if(CurIdxToken >= arrToken->len) {
        CurTok = (ARKToken){
                ._Type = TOKEN_EOF,
                ._Value = "<EOF>" // return end of file 
                                 // tell program to stop get token
            } ; // for debugging.
        return ; // break flow of this function.
    }
   
    // get element in arrToken by
    //   "g_array_index" is a function of Garray.
    //    use to get element in Garray.
    CurTok = (ARKToken) g_array_index(
                        arrToken , 
                        ARKToken , 
                        CurIdxToken++);
                        // => get Current index before 
                        // increasing it.
}

AST *newASTNode(ASTNodeType _type, ...){
    AST *tmpnode = (AST*) malloc(sizeof(tmpnode));
    tmpnode->_Type = _type;
    va_list args;
    va_start(args, _type);
    if(   _type == AST_ADD
        ||_type == AST_MINUS
        ||_type == AST_MUL
        ||_type == AST_DIV
    ){
        tmpnode->Binary.left = va_arg(args, AST* ); 
        strcpy(tmpnode->Binary.op, va_arg(args, char* )); 
        tmpnode->Binary.right = va_arg(args, AST* );
        return tmpnode;
    }

    
    switch(_type){
        case TOKEN_IDENTIFIER:
            tmpnode->Variable_name = va_arg(args,char*);
            break;
         
        default :
            break;
    }
    return tmpnode;
}


// A function create Abstruct Syntax Tree,
// (Parser).
/*
static int GetTokPrecedence() {
  if (!isascii(CurTok))
    return -1;

  // Make sure it's a declared binop.
  //int TokPrec = BinOpPrecedence[CurTok];
  if (TokPrec <= 0) return -1;
  return TokPrec;
}
*/

/*
    "pasre()" function's duty is handle parsing 
    and Creaing Abstruct Syntax Tree.
*/
AST* Parse(ARKTokenList * arrToken){
    initRoutine(arrToken);
    
    while(1){
        // Parsing the token.
        switch (CurTok._Type)
        {
            case TOKEN_IDENTIFIER:
                parseIdentifier();
                break;
            case TOKEN_LPAREN: // check if '('
                parseParen();
                break;
            case TOKEN_LBRACE: // check if '{' 
                break;
            case TOKEN_NUMBER :
                parseExpr();
                break;
            case TOKEN_EOF:
                // ........
                return ;
                
            default:
                return ;
        }
    }
    //return Created_AST;
}

AST *ParseIdentifier(){
    getNextToken();
    if(strcmp(CurTok._Value, "("))
        return 
}



AST *parseParen(){
    getNextToken(); // 

    AST * LHS = ParseExpr();
    if(!LHS) {
        logError("In ParseParen Memory error.");
        exit(-1);
    }
    if(strcmp(CurTok._Value, ")") != 0){
        fprintf(stderr , "expect \')'\n");
        exit(-1);
    }
    return LHS;
}

int main(){

}

