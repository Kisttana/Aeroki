#include "read_file.h"

char *read_file(const char * filepath){

     FILE* fp = fopen(filepath, "r"); // opening  file as a read-only file 
     
     if(fp == NULL) { // check if the file can't be opened up
          // printing error expect : No such file or directory.
          fprintf(stderr, "[error] : %s " ,strerror(errno));
          exit(EXIT_FAILURE);
     }
     
     fseek(fp,0, SEEK_END); // Set file indicator to the end of file (EOF)

     /* Set var len to the curren t position of file indicator 
      * (Set The length of file) 
      */
     size_t len = ftell(fp);      
     
     rewind(fp); // reset the file position indicator of the begining of file 

     char *buffer = (char*)malloc(len + 1); // +1 for null terminator

     // check if Memory Acllocation is not success.
     if (buffer != NULL) {
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
