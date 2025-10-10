#ifndef PARSER_H
#define PARSER_H

#include <glib.h>
#include <stddef.h>

/* lexer header must define:
   typedef GArray ARKTokenList;
   typedef <something> ARKToken;    
   typedef enum { ... } ARKTokenType;  // values like TOKEN_NUMBER, TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_COMMA, TOKEN_EOF, TOKEN_DEF, TOKEN_COMMA etc.
*/
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
    GArray *Args; /* GArray of char* */
    char *fnName;
} ASTFuncPrototype;

typedef struct FunctionNode {
    ASTFuncPrototype *proto;
    AST *body;
} ASTFunc;

typedef struct CallFunc {
    char *Callee;
    GArray *Args; /* GArray of AST* */
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

/* Parser context struct you defined */
typedef struct {
    ARKTokenList *tokens;
    size_t current_idx;
    struct Parser_context { ARKTokenType type; char *val; } ctx;
} ARKParser;

/* ---- Public API (kept the names you used) ---- */

/* initialize parser with token list (previously initRoutine) */
void initRoutine(ARKTokenList *tokens);

/* advance token - exposed in case other code expects it */
void getNextToken(void);

/* Parsing entry points (names kept from your code) */
AST *Parse( void ); /* behaves as ParsePrimary / expression entry (kept name) */
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
int getTokenPrec(const char *op);

/* For external debugging: set tokenlist manually (alias) */
void setTokenList(ARKTokenList *tokens);

#endif /* PARSER_H */

