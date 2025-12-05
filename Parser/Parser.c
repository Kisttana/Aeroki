#include "Parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


//#define PARSER_DEBUG

#ifdef PARSER_DEBUG

#include "../Lexer/print_tokentype.h"

#define dbg(__format,__context, ...) \
fprintf(stderr ,"[%s] :" __format"\n" , __context, __VA_ARGS__ ) 
#else 
 #define dbg(__format,__context, ...) 
#endif

/* ----------------------
   Module-scoped parser instance
   ---------------------- */
static ARKParser parser = {0};

/* Helper to read token from vector */ 
static inline ARKToken token_at(size_t idx) {
     dbg(" at %zu" ,"Get Token", idx);
     return vec_at(parser.tokens, ARKToken, idx);
}

/* initRoutine - initializes parser with token list */
void initRoutine(vector *tokens) {
     if (!tokens) {
          fprintf(stderr, "initRoutine: null token list\n");
          exit(EXIT_FAILURE);
     }


     parser.tokens = tokens;
     parser.current_idx = 0;
     /* set current ctx from first token */
     if (parser.tokens->size > 0) {
          ARKToken t = token_at(0);
          parser.ctx.type = t._Type;
          /* assume _Value is NUL-terminated */
          parser.ctx.val = strdup(t._Value);
     } else {
          parser.ctx.type = TOKEN_EOF; /* adapt token name if different */
          parser.ctx.val = strdup("<EOF>");
     }
     dbg(" %s [%zu] ,%s , %s" , "Parser Initialization" ,
         "successfully and set current to  ",
         0 , 
         typeof(parser.ctx.type) , 
         parser.ctx.val);
}

/* setTokenList alias */
void setTokenList(vector *tokens) { initRoutine(tokens); }

/* getNextToken - advances and updates parser.ctx */
void getNextToken(void) {
     if (!parser.tokens) {
          fprintf(stderr, "getNextToken: parser not initialized\n");
          exit(EXIT_FAILURE);
     }
     if (parser.current_idx + 1 >= parser.tokens->size) {
          /* set EOF-like token */
          parser.current_idx = parser.tokens->size;
          parser.ctx.type = TOKEN_EOF; /* adapt if necessary */
          free(parser.ctx.val);
          parser.ctx.val = strdup("<EOF>");

          dbg("%s","Get Next Token" , "the token is end");
          
          return;
     }
     parser.current_idx++;
     ARKToken t = token_at(parser.current_idx);
     parser.ctx.type = t._Type;
     free(parser.ctx.val);
     parser.ctx.val = strdup(t._Value);

     dbg("%s [%zu], %s, %s ", "Get Next Token" ,
                              "set token context to", 
                              parser.current_idx,
                              typeof(parser.ctx.type), 
                              parser.ctx.val);

}

/* Convenience to peek current token */
static inline ARKTokenType curTokenType(void) { return parser.ctx.type; }
static inline const char *curTokenVal(void) { return parser.ctx.val ? parser.ctx.val : ""; }

/* ----------------------
   AST constructors (typed, safe)
   ---------------------- */

static AST *newNumberNode(long double value) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_NUMBER;
     n->_Number = value;
     return n;
}

static AST *newIdentifierNode(const char *name) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_IDENTIFIER;
     n->Variable_name = strdup(name);
     return n;
}

static AST *newBinaryNode(const char *op, AST *left, AST *right) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_BINARY;
     n->Binary.left = left;
     n->Binary.right = right;
     n->Binary.op = strdup(op);
     return n;
}

static AST *newUnaryNode(const char *op, AST *operand) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_UNARY;
     n->Binary.left = NULL;
     n->Binary.right = operand;
     n->Binary.op = strdup(op);
     return n;
}

static AST *newCallNode(const char *callee, vector *args) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_CALL;
     n->Call.Callee = strdup(callee);
     n->Call.Args = args ? args : vec_init(0, AST*,0 );
     #ifdef PARSER_DEBUG
          dbg(" call => %s, args => [\n","[CallExpr Creater]", n->Call.Callee);
          for(size_t i = 0;i< n->Call.Args->size;i++) 
              printAST(vec_at(n->Call.Args,AST*,i),2);
          fprintf(stderr, "%s" , " ] \n"  );
     #endif
     return n;
}

static ASTFuncPrototype *newProto(const char *name, vector *args) {
     

     ASTFuncPrototype *p = calloc(1, sizeof(ASTFuncPrototype));
     p->fnName = strdup(name);
     p->Args = args ? args : vec_init(0, char*, 0);
     #ifdef PARSER_DEBUG
          dbg(" call => %s, args => [","[CallExpr Creater]", p->fnName);
          for(size_t i = 0;i< p->Args->size;i++) 
              printAST(vec_at(p->Args,AST*,i),2);
          fprintf(stderr, "%s" , " ] \n" );
     #endif
     return p;
}

static AST *newPrototypeNode(const char *name, vector *args) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_PROTOTYPE;
     n->Function.proto = newProto(name, args);
     return n;
}

static AST *newFunctionNode(ASTFuncPrototype *proto, AST *body) {
     AST *n = calloc(1, sizeof(AST));
     n->_Type = AST_FUNCTION;
     n->Function.proto = proto;
     n->Function.body = body;
     
     return n;
}

/* ----------------------
   Precedence helper
   ---------------------- */
int getTokenPrec(ARKTokenType op) {
     /* you can tune numeric values as needed */

     switch(op){
          case TOKEN_POW : return 700;

          // *(multiply) , %(modulo) , /(divided)
          case TOKEN_MUL :
          case TOKEN_MOD :
          case TOKEN_DIV : return 600;

          // +(Plus) , -(minus)
          case TOKEN_PLUS :
          case TOKEN_MINUS : return 500;

          /*
                < (Less than) ,
               <= (Less than or Equal),
                > (Greater than),
               >= (Greater that or Equal)
          */

          case TOKEN_LT :
          case TOKEN_LTE :
          case TOKEN_GT :
          case TOKEN_GTE : return 400;

          // == (Is Equal), != (Is Not Equal)
          case TOKEN_EQUAL_EQUAL :
          case TOKEN_NOT_EQUAL :  return 300;

          /*
               Token Logic AND => "and" (Modern-style) or "||" (C-style)
               Token Logic OR  =>  "or" (Modern-style) or "&&" (C-style)
          */
          case TOKEN_AND : return 200;
          case TOKEN_OR : return 100;


          case TOKEN_NON_OF_TOKEN :
          default : 
               return -1;
     }
}

/* ----------------------
   Parser: primary / unary / binop parsing
   ---------------------- */

/* Forward declarations (kept names similar to your code) */
AST *Parse(void); /* acts as primary entry */
AST *ParseExpression(void);
AST *ParseIdentifier(void);
AST *parseParen(void);
static AST *parseNumberInternal(void);
static AST *parsePrimaryInternal(void);
static AST *parseUnaryInternal(void);
static AST *parseBinOpRHSInternal(int exprPrec, AST *lhs);

/* parse number */
static AST *parseNumberInternal(void) {
     /* Assumes cur token is a number and string in ctx */
     long double val = strtold(curTokenVal(), NULL);
     AST *n = newNumberNode(val);
     getNextToken();
     return n;
}

/* parse identifier or function call */
AST *ParseIdentifier(void) {
     /* current token must be an identifier */
     const char *name = curTokenVal();
     
     printf("in parse iden : %s\n", name);

     char namebuf[256];
     strncpy(namebuf, name, sizeof(namebuf)-1);
     namebuf[sizeof(namebuf)-1] = '\0';
     getNextToken(); /* eat identifier */

     /* if next is '(' -> function call */
     if (curTokenType() != TOKEN_LPAREN) {
          return newIdentifierNode(namebuf);
     }

     /* parse call args */
     getNextToken(); /* eat '(' */
     vector *args = vec_init(0,AST*,0);;
     if (curTokenType() != TOKEN_RPAREN) {
          while (1) {
               AST *arg = ParseExpression();
               if (!arg) {
                    /* cleanup args */
                    for (size_t i = 0; i < args->size; ++i) 
                         freeAST(vec_at(args, AST*, i));
                    vec_clean(args);
                    return NULL;
               }
               vec_push_back(args,AST*, arg);
               if (curTokenType() == TOKEN_RPAREN) break;
               if (curTokenType() != TOKEN_COMMA) {
                    fprintf(stderr, "ParseIdentifier: expected ',' or ')'\n");
                    for (size_t i = 0; i < args->size; ++i) 
                         freeAST(vec_at(args, AST* , i));
                    vec_clean(args);
                    return NULL;
               }
               getNextToken(); /* eat comma */
          }
     }
     getNextToken(); /* eat ')' */
     return newCallNode(namebuf, args);
}

/* parse parentheses */
AST *parseParen(void) {

     getNextToken(); /* eat '(' */
     AST *v = ParseExpression();
     //if (!v) return NULL;
     if (curTokenType() != TOKEN_RPAREN) {
          fprintf(stderr, "parseParen: expected ')'\n");
          freeAST(v);
          return NULL;
     }
     getNextToken(); /* eat ')' */
     return v;
}

/* primary (number | identifier | paren) - internal */
static AST *parsePrimaryInternal(void) {
     switch(curTokenType()){
          case TOKEN_EOF: return NULL;
          case TOKEN_IDENTIFIER : return ParseIdentifier();
          case TOKEN_NUMBER : return parseNumberInternal();
          case TOKEN_LPAREN : return parseParen();
          
          default :
               fprintf(stderr, "parsePrimary: unexpected token '%s'\n", curTokenVal());
               return NULL;
     }
}

/* unary: handle unary operators like -x or !x */
static AST *parseUnaryInternal(void) {
     ARKTokenType type = curTokenType();
     if (type == TOKEN_MINUS || type == TOKEN_BANG) {
          char opbuf[8];
          strncpy(opbuf, curTokenVal(), sizeof(opbuf)-1);
          opbuf[sizeof(opbuf)-1] = '\0';
          getNextToken();
          AST *operand = parseUnaryInternal();
          if (!operand) return NULL;
          return newUnaryNode(opbuf, operand);
     }
     return parsePrimaryInternal();
}

/* binary RHS parsing (classic precedence climbing) */
static AST *parseBinOpRHSInternal(int exprPrec, AST *lhs) {
     while (1) {

          int tokPrec = getTokenPrec(curTokenType());
          if (tokPrec < exprPrec) return lhs;

          /* operator */
          char opbuf[8];
          strncpy(opbuf, curTokenVal(), sizeof(opbuf)-1);
          opbuf[sizeof(opbuf)-1] = '\0';
          getNextToken(); /* eat operator */

          AST *rhs = parseUnaryInternal();
          if (!rhs) {
               freeAST(lhs);
               return NULL;
          }

          /* If next operator has higher precedence, it binds to rhs */
          int nextPrec = getTokenPrec(curTokenType());
          if (tokPrec < nextPrec) {
               rhs = parseBinOpRHSInternal(tokPrec + 1, rhs);
               if (!rhs) {
                    freeAST(lhs);
                    return NULL;
               }
          }

          lhs = newBinaryNode(opbuf, lhs, rhs);
     }
}

/* ParseExpression (exposed) */
AST *ParseExpression(void) {
     AST *lhs = parseUnaryInternal();
     if (!lhs) return NULL;
     return parseBinOpRHSInternal(0, lhs);
}

/* Parse API */
AST *Parse(void) {
     return parsePrimaryInternal();
}

/* ----------------------
   Prototype / Definition / Top-level
   ---------------------- */

/* Parse prototype: identifier '(' arg1, arg2 ')' */
AST *ParsePrototype(void) {
     if (curTokenType() != TOKEN_IDENTIFIER) {
          fprintf(stderr, "ParsePrototype: expected identifier\n");
          return NULL;
     }
     char namebuf[256];
     strncpy(namebuf, curTokenVal(), sizeof(namebuf)-1);
     namebuf[sizeof(namebuf)-1] = '\0';
     getNextToken(); /* eat name */

     if (curTokenType() != TOKEN_LPAREN) {
          fprintf(stderr, "ParsePrototype: expected '('\n");
          return NULL;
     }
     getNextToken(); /* eat '(' */

     vector *args = vec_init(0,char *,0);
     while (curTokenType() == TOKEN_IDENTIFIER) {
          char *a = strdup(curTokenVal());
          vec_push_back(args,char*, a);
          getNextToken();
          if (curTokenType() == TOKEN_COMMA) getNextToken();
          else break;
     }
     if (curTokenType() != TOKEN_RPAREN) {
          fprintf(stderr, "ParsePrototype: expected ')'\n");
          for (size_t i = 0; i < args->size; ++i) free(vec_at(args, char *, i));
          vec_clean(args);
          return NULL;
     }
     getNextToken(); /* eat ')' */
     return newPrototypeNode(namebuf, args);
}

/* ParseDefinition: 'def' prototype expression */
AST *ParseDefinition(void) {
     if (curTokenType() != TOKEN_DEF) {
          fprintf(stderr, "ParseDefinition: expected 'def'\n");
          return NULL;
     }
     getNextToken(); /* eat def */
     AST *proto = ParsePrototype();
     if (!proto) return NULL;
     ASTFuncPrototype *pp = proto->Function.proto;
     free(proto); /* proto AST wrapper no longer needed */
     AST *body = ParseExpression();
     if (!body) {
          /* cleanup pp resources */
          for (size_t i = 0; i < pp->Args->size; ++i) free(vec_at_ptr(pp->Args, char *, i));
          vec_clean(pp->Args);
          free(pp->fnName);
          free(pp);
          return NULL;
     }
     return newFunctionNode(pp, body);
}

static void indent_print(int depth) {
     for (int i=0;i<depth;++i) putchar(' ');
}

void ast_parse(ASTProgram *__context__, const char *__module__, vector *tokens) {
     
     initRoutine(tokens);

     // set Module name to context
     strncpy((__context__)->ModuleName, 
             __module__,
             128
     );
     
     __context__->ASTExprVector = vec_init(0, AST*, 0);

     AST *expr = NULL;
     while(1){
         expr = ParseExpression();
          if(!expr) {
               dbg("%s", "Make AST", "Parsing Program is complete.\n");
               break;
          }

          vec_push_back(__context__->ASTExprVector, AST*, expr);
     }
     __context__->ASTExprCount = __context__->ASTExprVector->size;

}


void print_ast_structure(ASTProgram *ast){
     printf("(Module : %s) => [ \n",ast->ModuleName);
     for(size_t i = 0;i< ast->ASTExprCount;++i){
          printAST(vec_at(ast->ASTExprVector,AST*,i),5);
          indent_print(5);
          puts(",");
     }

     puts("\n]( end )");
}



/* ----------------------
   AST printing and freeing
   ---------------------- */



void printAST(const AST *node, int depth) {
     if (!node) {
          indent_print(depth); printf("(NULL)\n"); return;
     }
     switch (node->_Type) {
          case AST_NUMBER:
               indent_print(depth); printf("(NUM %Lf)\n", node->_Number);
               break;
          case AST_IDENTIFIER:
               indent_print(depth); printf("(IDENT %s)\n", node->Variable_name);
               break;
          case AST_UNARY:
               indent_print(depth); printf("(UNARY %s\n", node->Binary.op);
               printAST(node->Binary.right, depth+2);
               indent_print(depth); printf(")\n");
               break;
          case AST_BINARY:
               indent_print(depth); printf("(BIN %s\n", node->Binary.op);
               printAST(node->Binary.left, depth+2);
               printAST(node->Binary.right, depth+2);
               indent_print(depth); printf(")\n");
               break;
          case AST_CALL:
               indent_print(depth); printf("(CALL %s\n", node->Call.Callee);
               for (size_t i=0;i<node->Call.Args->size;++i) printAST(vec_at(node->Call.Args, AST*, i), depth+2);
               indent_print(depth); printf(")\n");
               break;
          case AST_PROTOTYPE:
               indent_print(depth); printf("(PROTO %s (", node->Function.proto->fnName);
               for (size_t i=0;i<node->Function.proto->Args->size;++i) {
                    printf("%s", vec_at(node->Function.proto->Args, char*, i));
                    if (i+1 < node->Function.proto->Args->size) printf(", ");
               }
               printf("))\n");
               break;
          case AST_FUNCTION:
               indent_print(depth); printf("(FUNC %s\n", node->Function.proto->fnName);
               printAST(node->Function.body, depth+2);
               indent_print(depth); printf(")\n");
               break;
          default:
               indent_print(depth); printf("(UNKNOWN)\n");
     }
}

/* free A:ST (mirror constructors) */
void freeAST(AST *node) {
     if (!node) return;
     switch(node->_Type) {
          case AST_NUMBER:
               free(node);
               return;
          case AST_IDENTIFIER:
               free(node->Variable_name);
               free(node);
               return;
          case AST_UNARY:
               free(node->Binary.op);
               freeAST(node->Binary.right);
               free(node);
               return;
          case AST_BINARY:
               free(node->Binary.op);
               freeAST(node->Binary.left);
               freeAST(node->Binary.right);
               free(node);
               return;
          case AST_CALL:
               free(node->Call.Callee);
               for (size_t i=0;i<node->Call.Args->size;++i) freeAST(vec_at(node->Call.Args, AST*, i));
               vec_clean(node->Call.Args);
               free(node);
               return;
          case AST_PROTOTYPE:
               for (size_t i=0;i<node->Function.proto->Args->size;++i) free(vec_at_ptr(node->Function.proto->Args, char*, i));
               vec_clean(node->Function.proto->Args);
               free(node->Function.proto->fnName);
               free(node->Function.proto);
               free(node);
               return;
          case AST_FUNCTION:
               freeAST(node->Function.body);
               for (size_t i=0;i<node->Function.proto->Args->size;++i) free(vec_at_ptr(node->Function.proto->Args, char*, i));
               vec_clean(node->Function.proto->Args);
               free(node->Function.proto->fnName);
               free(node->Function.proto);
               free(node);
               return;
          default:
               free(node);
               return;
     }
}

