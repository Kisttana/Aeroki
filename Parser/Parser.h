#ifndef PARSER_H
#define PARSER_H


/* lexer header must define:
   typedef vector vector;
   typedef <something> ARKToken;    
   typedef enum { ... } ARKTokenType;  // values like TOKEN_NUMBER, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_COMMA, TOKEN_EOF, TOKEN_DEF, TOKEN_COMMA etc.
*/
#include "../libs/Aegis/include/aegis.h"
#include "../Lexer/Lexer.h" /* adapt path/name if needed */


/* AST node types (match your previous names or adapt) */
typedef enum {
    AST_NUMBER,
    AST_IDENTIFIER,
    AST_BINARY,
    AST_UNARY,
    AST_CALL,
    AST_PROTOTYPE,
    AST_FUNCTION,
    AST_UNKNOWN
} ASTNodeType;

/* Forward declaration */
typedef struct AbstractSyntaxTree AST;

/* AST structures - same layout you used earlier */
typedef struct BinaryNode {
    AST *left;
    char *op;
    AST *right;
} ASTBinary;

typedef struct PrototypeNode {
    vector *Args; /* GArray of char* */
    char *fnName;
} ASTFuncPrototype;

typedef struct FunctionNode {
    ASTFuncPrototype *proto;
    AST *body;
} ASTFunc;

typedef struct CallFunc {
    char *Callee;
    vector *Args; /* GArray of AST* */
} ASTCall;

struct AbstractSyntaxTree {
    ASTNodeType _Type;
    union {
        long double _Number;
        char *Variable_name;
        ASTBinary Binary;
        ASTFunc Function;
        ASTCall Call;
    };
};

/* Parser context struct definition*/
typedef struct {
    vector *tokens;
    size_t current_idx;
    struct Parser_context { ARKTokenType type; char *val; } ctx;
} ARKParser;


/* initialize parser with token list (previously initRoutine) */
void initRoutine(vector *tokens);

/* advance token - exposed in case other code expects it */
void getNextToken(void);

/* Parsing entry points (names kept from your code) */
AST *Parse( void );  
AST *ParseExpression(void);
AST *ParseTopLevelExpr(void);
AST *ParsePrototype(void);
AST *ParseDefinition(void);
AST *ParseIdentifier(void);
AST *parseParen(void);

/* Debug / helpers */
void printAST(const AST *node, int depth);
void freeAST(AST *node);

/* Utility to map operator strings to precedence */
int getTokenPrec(ARKTokenType op);

/* For external debugging: set tokenlist manually (alias) */
void setTokenList(vector *tokens);


typedef struct { 
     char ModuleName[128];
     size_t ASTExprCount;
     vector *ASTExprVector;
}ASTProgram;


void ast_parse(ASTProgram *__context__, const char *__module__, vector *tokens);
void print_ast_structure(ASTProgram *ast);

#endif // PARSER_H 

