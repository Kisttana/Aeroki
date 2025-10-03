#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h> // Memory Management malloc() , free() .
#include <stdarg.h> // Variadic functions.
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
    AST_VAR,
    AST_BLOCK, // Function block
    AST_ADD, // Operater Plus => "+"
    AST_MINUS, // Operator Minus => "-"
    AST_MUL, // Operater Multiply => "*"
    AST_POW, // Operater Power =>  o"**"
    AST_BIT_NOT, // Operater Bitwise Not => "~"
    AST_BIT_AND, //Operater Bitwise AND => "&"
    AST_BIT_OR, //Operater Bitwise AND => "|"
    AST_BIT_SHIFTL ,// Operater  => "<<" (shift Left)
    AST_BIT_SHIFTR , // OPerater => ">>" (shift Right)

    AST_CALL // Function Call => F(arg1 , arg2, ...) 
}ASTNodeType;


typedef struct {
    struct ASTNode *left;
    char op[3];
    struct ASTNode *right;
} ASTBinaryNode;


typedef struct ASTNode {
    ASTNodeType _Type;
    union {
        long double _Number;
        char *Variable_name;
        ASTBinaryNode Binary; 
    };
} AST;


AST *newASTNode(ASTNodeType _type, ...);
AST *Parse(ARKTokenList *TokList);
AST *ParseIdentifierExpr();
AST *ParseNumber();
AST *ParseParen();
AST *ParseExpr
#endif // PARSER_H