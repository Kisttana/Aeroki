/*
     author: Piyaphat Jaiboon
     contact: 
            email: fiw.contact.work@gmail.com
            github: https://github.com/Piyaph4t 
     date: June 10, 2024
     description: Implementation of the Lexer for tokenizing source code.
*/

#include "Lexer.h"
#include "read_file.h"
#include "../../Internal/ark_mem.h" //< for memory allocation macros

#define SPACE ' '

ArkLexer *_Ark_ScanTokens(const char *file_to_scan)
{
          
     

     //< Initialization section >
     ArkLexer *NewLexer;                              //< Pointer to new ArkLexer instance.   
     ArkToken token = {0};                            //< Temporary token for storing each lexeme's information.
     uint32_t line = 1;                               //< Current line number in the source file.  
     char *letter;                                    //< Pointer to current letter in source string.
     int (*condition)(int) = NULL;                    //< Function pointer for determining How to read the token.
     int isNewline = 0;                               //< Flag to check newline character.    
     ArkTokenType type = UNKNOWN_TOKEN;               //< Set type to UNKNOWN_TOKEN initially.
     

     NewLexer =  NEW(ArkLexer);                        //< allocate memory 
     NewLexer->token_list = vec_init(0, ArkToken, 0);  //< allocate memory for token list
     NewLexer->file = strdup(file_to_scan);            //< copy file_to_scan to file_name 
     NewLexer->buffer = read_file(NewLexer->file);     //< Read the file and store it into buffer.
     NewLexer->begin = NewLexer->buffer;               //< Set begin to the begining of the buffer string.
     NewLexer->cursor = NewLexer->buffer;              //< Set cursor to the current position of buffer 

     //< end initialization section >

     /* Main scanning loop
      * Scan until the end of the buffer string <cursor>
      * Each iteration processes one token
      * then the NewLexer->cursor is updated by each reading token function.
     */
     while ( *NewLexer->cursor != '\0' ) {

          letter = NewLexer->cursor; //< Access current character pointed by cursor

          //< check if it's newline or space(' ') 

          //< Set isNewline to logic condition
          //< If it's true , isNewline is one and increase line number, otherwise zero.
          isNewline = (letter[0] == '\n');
          if (isNewline){
               line++;
               token.type = NL;
               token.line = line;
               strcpy(token.lexeme, "\\n");
               vec_push_back(NewLexer->token_list, ArkToken, token); //< Append token to
               NewLexer->cursor++; //< Move cursor to next character.
               NewLexer->begin = NewLexer->cursor; //< Update begin to current cursor position.
               continue;
          }
          

          if ( letter[0] == SPACE) {
               ++NewLexer->cursor; //< Move cursor to next character.
               NewLexer->begin = NewLexer->cursor; //< Update begin to current cursor position.
               continue;
          }
          
          
          condition = NULL;           //< Set condition to NULL for control flow of program. 
          isNewline = 0;              //< Reset isNewline every loop to wait checking the next Newline character. 
          type = UNKNOWN_TOKEN;       //< Set type to UNKNOWN_TOKEN for debugging purposes.
          memset(&token,'\0', sizeof(ArkToken)); //< Reset token struct every loop.

          /* Determine how to read the token based on the first character.
           * Call the appropriate reading function.
           * 
          */
          if(  letter[0] == '\"')
              _Ark_ReadString(&NewLexer->cursor, &token, &type);

          else if ( isalpha(letter[0]) ) { 
               condition = _Ark_isIden;
               type = NAME;
          }
          else if ( isdigit(letter[0]) ) {
               condition = _Ark_isNumber;

               type = NUMBER;
          }
          else if ( ispunct(letter[0]) ) 
               _Ark_ReadPunct(&NewLexer->cursor, &token, &type);
          
          else {
               fprintf(stderr,"reconized error");
               exit(EXIT_FAILURE);

               vec_clean(NewLexer->token_list);
               free(NewLexer);
          }
          //< If condition is set, read the token using the specified condition function.
          if( condition != NULL)
               _Ark_ReadToken(token.lexeme, &NewLexer->cursor, condition);
          
          token.line = line;
          token.type = type;
          NewLexer->begin = NewLexer->cursor; //< Update begin to current cursor position.
          vec_push_back(NewLexer->token_list, ArkToken, token); //< Append token to the token list.
     }
     //< Append EOF token at the end of token list.
     ArkToken eof = {.type = EOF_TOKEN, .lexeme = "<EOF>", .line = line};
     vec_push_back(NewLexer->token_list, ArkToken, eof);


     return NewLexer;
}
