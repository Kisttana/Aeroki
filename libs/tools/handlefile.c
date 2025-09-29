#include "handlefile.h"
#include "print_error.h"

char *read_file2string(const char * filepath){
	FILE* fp = fopen(filepath, "r");
	if(fp == NULL) {
		printf("\nRead file error \n");	
		exit(1);
	}

	fseek(fp,0, SEEK_END);
    size_t _len = ftell(fp);
    rewind(fp);

    char *buffer = (char*)malloc(_len + 1); // +1 for null terminator
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    fread(buffer, 1, _len, fp);
	
	if(!buffer) {
		perror("fread\n");
	}
	buffer[_len] = '\0'; // Null-terminate for convenience

    fclose(fp);
    return buffer;
}
