#define _GNU_SOURCE
#include "Parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ----------------------
   Module-scoped parser instance
   ---------------------- */
static ARKParser parser = {0};

/* Helper to read token from GArray - adapt if your ARKTokenList is different */
static inline ARKToken token_at(size_t idx) {
    return g_array_index(parser.tokens, ARKToken, idx);
}

/* initRoutine - initializes parser with token list (keeps name) */
void initRoutine(ARKTokenList *tokens) {
    if (!tokens) {
        fprintf(stderr, "initRoutine: null token list\n");
        exit(EXIT_FAILURE);
    }
    parser.tokens = tokens;
    parser.current_idx = 0;
    /* set current ctx from first token */
    if (parser.tokens->len > 0) {
        ARKToken t = token_at(0);
        parser.ctx.type = t._Type;
        /* assume _Value is NUL-terminated */
        parser.ctx.val = strdup(t._Value);
    } else {
        parser.ctx.type = TOKEN_EOF; /* adapt token name if different */
        parser.ctx.val = strdup("<EOF>");
    }
}

/* setTokenList alias */
void setTokenList(ARKTokenList *tokens) { initRoutine(tokens); }

/* getNextToken - advances and updates parser.ctx */
void getNextToken(void) {
    if (!parser.tokens) {
        fprintf(stderr, "getNextToken: parser not initialized\n");
        exit(EXIT_FAILURE);
    }
    if (parser.current_idx + 1 >= parser.tokens->len) {
        /* set EOF-like token */
        parser.current_idx = parser.tokens->len;
        parser.ctx.type = TOKEN_EOF; /* adapt if necessary */
        free(parser.ctx.val);
        parser.ctx.val = strdup("<EOF>");
        return;
    }
    parser.current_idx++;
    ARKToken t = token_at(parser.current_idx);
    parser.ctx.type = t._Type;
    free(parser.ctx.val);
    parser.ctx.val = strdup(t._Value);
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

static AST *newCallNode(const char *callee, GArray *args) {
    AST *n = calloc(1, sizeof(AST));
    n->_Type = AST_CALL;
    n->Call.Callee = strdup(callee);
    n->Call.Args = args ? args : g_array_new(FALSE, FALSE, sizeof(AST *));
    return n;
}

static ASTFuncPrototype *newProto(const char *name, GArray *args) {
    ASTFuncPrototype *p = calloc(1, sizeof(ASTFuncPrototype));
    p->fnName = strdup(name);
    p->Args = args ? args : g_array_new(FALSE, FALSE, sizeof(char *));
    return p;
}

static AST *newPrototypeNode(const char *name, GArray *args) {
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
int getTokenPrec(const char *op) {
    /* you can tune numeric values as needed */
    if (!op) return -1;
    if (strcmp(op, "**") == 0) return 70;
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0 || strcmp(op, "%") == 0) return 60;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0) return 50;
    if (strcmp(op, "<") == 0 || strcmp(op, "<=") == 0 || strcmp(op, ">" )==0 || strcmp(op, ">=")==0) return 40;
    if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0) return 30;
    if (strcmp(op, "&&") == 0) return 20;
    if (strcmp(op, "||") == 0) return 10;
    return -1;
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
    GArray *args = g_array_new(FALSE, FALSE, sizeof(AST *));
    if (curTokenType() != TOKEN_RPAREN) {
        while (1) {
            AST *arg = ParseExpression();
            if (!arg) {
                /* cleanup args */
                for (guint i = 0; i < args->len; ++i) freeAST(g_array_index(args, AST *, i));
                g_array_free(args, TRUE);
                return NULL;
            }
            g_array_append_val(args, arg);
            if (curTokenType() == TOKEN_RPAREN) break;
            if (curTokenType() != TOKEN_COMMA) {
                fprintf(stderr, "ParseIdentifier: expected ',' or ')'\n");
                for (guint i = 0; i < args->len; ++i) freeAST(g_array_index(args, AST *, i));
                g_array_free(args, TRUE);
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
    if (!v) return NULL;
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
    if (curTokenType() == TOKEN_IDENTIFIER) {
        return ParseIdentifier();
    }
    if (curTokenType() == TOKEN_NUMBER) {
        return parseNumberInternal();
    }
    if (curTokenType() == TOKEN_LPAREN) {
        return parseParen();
    }
    fprintf(stderr, "parsePrimary: unexpected token '%s'\n", curTokenVal());
    return NULL;
}

/* unary: handle unary operators like -x or !x */
static AST *parseUnaryInternal(void) {
    if (curTokenType() == TOKEN_MINUS || curTokenType() == TOKEN_BANG) {
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
        const char *tokOp = curTokenVal();
        int tokPrec = getTokenPrec(tokOp);
        if (tokPrec < exprPrec) return lhs;

        /* operator */
        char opbuf[8];
        strncpy(opbuf, tokOp, sizeof(opbuf)-1);
        opbuf[sizeof(opbuf)-1] = '\0';
        getNextToken(); /* eat operator */

        AST *rhs = parseUnaryInternal();
        if (!rhs) {
            freeAST(lhs);
            return NULL;
        }

        /* If next operator has higher precedence, it binds to rhs */
        int nextPrec = getTokenPrec(curTokenVal());
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

/* Parse (kept name: acts like primary entry for legacy code) */
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

    GArray *args = g_array_new(FALSE, FALSE, sizeof(char *));
    while (curTokenType() == TOKEN_IDENTIFIER) {
        char *a = strdup(curTokenVal());
        g_array_append_val(args, a);
        getNextToken();
        if (curTokenType() == TOKEN_COMMA) getNextToken();
        else break;
    }
    if (curTokenType() != TOKEN_RPAREN) {
        fprintf(stderr, "ParsePrototype: expected ')'\n");
        for (guint i = 0; i < args->len; ++i) free(g_array_index(args, char *, i));
        g_array_free(args, TRUE);
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
        for (guint i = 0; i < pp->Args->len; ++i) free(g_array_index(pp->Args, char *, i));
        g_array_free(pp->Args, TRUE);
        free(pp->fnName);
        free(pp);
        return NULL;
    }
    return newFunctionNode(pp, body);
}

/* ParseTopLevelExpr: wrap expression into anonymous function */
AST *ParseTopLevelExpr(void) {
    AST *e = ParseExpression();
    if (!e) return NULL;
    ASTFuncPrototype *proto = newProto("__anon_expr", NULL);
    return newFunctionNode(proto, e);
}

/* ----------------------
   AST printing and freeing
   ---------------------- */

static void indent_print(int depth) {
    for (int i=0;i<depth;++i) putchar(' ');
}

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
            for (guint i=0;i<node->Call.Args->len;++i) printAST(g_array_index(node->Call.Args, AST*, i), depth+2);
            indent_print(depth); printf(")\n");
            break;
        case AST_PROTOTYPE:
            indent_print(depth); printf("(PROTO %s (", node->Function.proto->fnName);
            for (guint i=0;i<node->Function.proto->Args->len;++i) {
                printf("%s", g_array_index(node->Function.proto->Args, char*, i));
                if (i+1 < node->Function.proto->Args->len) printf(", ");
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

/* free AST (mirror constructors) */
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
            for (guint i=0;i<node->Call.Args->len;++i) freeAST(g_array_index(node->Call.Args, AST*, i));
            g_array_free(node->Call.Args, TRUE);
            free(node);
            return;
        case AST_PROTOTYPE:
            for (guint i=0;i<node->Function.proto->Args->len;++i) free(g_array_index(node->Function.proto->Args, char*, i));
            g_array_free(node->Function.proto->Args, TRUE);
            free(node->Function.proto->fnName);
            free(node->Function.proto);
            free(node);
            return;
        case AST_FUNCTION:
            freeAST(node->Function.body);
            for (guint i=0;i<node->Function.proto->Args->len;++i) free(g_array_index(node->Function.proto->Args, char*, i));
            g_array_free(node->Function.proto->Args, TRUE);
            free(node->Function.proto->fnName);
            free(node->Function.proto);
            free(node);
            return;
        default:
            free(node);
            return;
    }
}

