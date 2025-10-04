#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h> // Memory Management malloc() , free() .
#include <stdarg.h> // Variadic functions.
#include <stdbool.h> // true and false
#include "../libs/tools/print_error.h"
/* 
    -For showing Log.
    -Tell User syntax error.
*/



#include "../Lexer/Lexer.h"
/*
    Include Lexer.h for 
    -ARKToken => struct define in Lexer.h
                 for containing and managing
                 Token.

    -ARKTokenList * (Pointer of ARKTokenList) 
        => implement from Garray that is Datastructure
        from GLIB maintain by GNOME .
        
        Using It to contain List of ARKToken ,
        which Generate by "scanLexer" in **Lexer.h**.

        Note "C" language doesn't have Standard Template Library like "C++",

        It's a Alternative of std::vector .
    
    -ARKLexer
    
*/

typedef enum{
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_BLOCK, // Function block
    AST_BINARY,
    AST_PROTOTYPE, // Function declearation 
    AST_CALL // Function Call => F(arg1 , arg2, ...) 
}ASTNodeType;


typedef struct {
    struct ASTNode *left;
    char op[3];
    struct ASTNode *right;
} ASTBinary;


typedef struct Prototype {
    GArray *Args;
    char *fnName;
}ASTFuncPrototype;

typedef struct Function{
    ASTFuncPrototype *proto;
    AST *body;
}ASTFunc;

typedef struct CallFunc{
    char *Callee;
    GArray *Args;
}ASTCall;

typedef struct AbstractSyntaxTree{
    ASTNodeType _Type;
    union {
        long double _Number;
        char *Variable_name;
        ASTBinary Binary; 
        ASTFunc Function;
    };
} AST;


AST *newASTNode(ASTNodeType _type, ...);
AST *Parse();
AST *ParseIdentifier();
AST *ParseNumber();
AST *ParseParen();
AST *ParseExpression();
#endif // PARSER_H