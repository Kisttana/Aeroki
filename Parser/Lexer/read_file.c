/*
     author: Piyaphat Jaiboon
     contact: 
            email: fiw.contact.work@gmail.com
            github: https://github.com/Piyaph4t 
     date: June 10, 2024
     description: Implementation of the Lexer for tokenizing source code.
*/

#include "read_file.h"

char *read_file(const char *file_path){
     
     FILE *fp = fopen(file_path,"r");
     if(fp == NULL) { // check if the file can't be opened up
          // printing error expect : No such file or directory.
          perror("ark : can't open file ");
          exit(EXIT_FAILURE);
     }
     
     fseek(fp,0, SEEK_END); // Set file indicator to the end of file (EOF)

     /* Set len to the current position of file indicator
      * (In this case, it's the size of file in bytes) 
      */
     size_t len = ftell(fp);      
     
     rewind(fp); // reset the file position indicator of the begining of file 

     char *buffer = (char*)malloc(len + 1); // +1 for null terminator


     // check if Memory Acllocation is not success.
     if (buffer == NULL) {
          perror("malloc\n");
          exit(EXIT_FAILURE);
     }
     // Reading File into buffer
     fread(buffer, 1, len, fp);

     // check if Memory Acllocation is not success.
     if(buffer == NULL){
          perror("fread\n");
          exit(EXIT_FAILURE);
     }

     buffer[len] = '\0'; // Setting Null-terminate 

     fclose(fp); // Closing File

     return buffer;
}
