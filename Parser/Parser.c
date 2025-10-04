#include "Parser.h" // AST , parse()

/*
    This make a managing Rountine to handle
    Token after generate from Lexer.
*/
static size_t CurIdxToken = 0; 
static ARKToken CurTok;
static ARKTokenList *arrToken = NULL;

// will be an alternative way for optimization.
//static GHashTable BinopPrecedence; 

void initRoutine(ARKTokenList *tokens) {arrToken = tokens;}

static int getTokenPrec(const char * op){
    char *precedence[] = {"-" , "+", "*" , "/" , "**"};
    for(int i=0;i<5;++i)
    if(!strcmp(op,precedence[i])) 
        return i;
    return -1;
}


static ARKTokenType getNextToken(){
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
    return CurTok._Type;
}

AST *newASTNode(ASTNodeType _type, ...){
    // Reserve Memory
    AST *newnode = (AST*) malloc(sizeof(AST));
    newnode->_Type = _type; // Set type of AST node
    
    va_list args;
    va_start(args, _type);
   
    strcpy(newnode->Binary.op, va_arg(args, char* )); 
    newnode->Binary.left = va_arg(args, AST* ); 
    newnode->Binary.right = va_arg(args, AST* );

    switch(_type){
        case AST_PROTOTYPE:
                // Set function name
            newnode->Function.proto->fnName = va_arg(
                                                    args,
                                                    char*);
                
                // Set function's arguments;
            newnode->Function.proto->Args =  va_arg(
                                                    args,
                                                    GArray*);
            break;
        case AST_BINARY:
            strcpy(newnode->Binary.op, va_arg(args, char* )); 
            newnode->Binary.left = va_arg(args, AST* ); 
            newnode->Binary.right = va_arg(args, AST* );
        default :
            logError("Couldn't recognize the type");
            exit(-1);
            break;
    }
    
    return newnode;
}




/*
    "pasre()" function's duty is handle parsing 
    and Creaing Abstruct Syntax Tree.
*/
AST* Parse(){
  
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
                ParseNumber();
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
AST *ParseExpression() {
  auto LHS = Parse();
  if (!LHS)
    return NULL;

  return ParseBinOpRHS(0, LHS);
}
AST *ParseBinOpRHS(int ExprPrec, AST *LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    char *BinOp = CurTok._Value;
    getNextToken(); // eat binop

    // Parse the primary expression after the binary operator.
    AST *RHS = Parse();
    if (!RHS)
      return NULL;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, RHS);
      if (!RHS)
        return NULL;
    }

    // Merge LHS/RHS.
    LHS =
        newASTNode(AST_BINARY, BinOp, LHS, RHS);
  }
}


AST *ParseIdentifier(){
    // Keep Identifier
    const char *IdentifierStr = CurTok._Value;
    
    
    getNextToken(); // eat Identifier 
    
    // check if it's literally Variable
    if(!strcmp(IdentifierStr, "("))
        return newASTNode(AST_IDENTIFIER, CurTok._Value);
    
    getNextToken();// eat '('

    //if it's calling function 
    AST *Arg;
    GArray *Args = g_array_new(false, false, sizeof(AST*));
    if (strcmp(CurTok._Value, ")") != 0) {
        while (true) {
            if ( Arg = ParseExpression())
                g_array_append_val(Args, Arg);
            else
                return NULL;

            if (!strcmp(CurTok._Value, ")"))  {
                getNextToken();
                break;
            }

            if (!strcmp(CurTok._Value, ",")){
                LogError("Expected ')' or ',' in argument list");
                exit(-1);
            }
        
            getNextToken();
        }   
    }
    return newASTNode(AST_CALL, IdentifierStr, Args); 
}
AST *ParseNumber(){
    AST* NumberNode = newASTNode(AST_NUMBER, strtold(CurTok._Value));
    getNextToken();
    return NumberNode;
}


AST *parseParen(){
    getNextToken(); // 

    AST * LHS = ParseExpr();
    if(!LHS)
        return NULL;
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



AST *ParsePrototype() {
  if (CurTok._Type != TOKEN_IDENTIFIER)
    return LogErrorP("Expected function name in prototype");

  char *FnName = CurTok._Value;
  getNextToken();

  if (strcmp(CurTok._Value, "("))
    return LogErrorP("Expected '(' in prototype");

  // Read the list of argument names.
  GArray *ArgsName = g_array_new(false, false , sizeof(char*));
  while (getNextToken() == TOKEN_IDENTIFIER) 
        g_array_append_val(ArgsName, CurTok._Value);

  if (strcmp(CurTok._Value, ")"))
    return LogErrorP("Expected ')' in prototype");

  // success.
  getNextToken();  // eat ')'.

  return newASTNode(AST_PROTOTYPE , FnName, ArgsName);
}


