#include "Handlefile.h"
#include "print_error.h"

char *read_file2string(const char * filepath){
	FILE* fp = fopen(filepath, "r");
	if(!fp) {
		ARKprint_error(errno);
		exit(errno);
	}

	fseek(f,r 0, SEEK_END);
    size_t _len = ftell(fp);
    rewind(fp);

    char *buffer = (char*)malloc(_len + 1); // +1 for null terminator
    if (!buffer) {
        perror("malloc");
        exit(1);
    }

    fread(buffer, 1, _len, fp);
    buffer[_len] = '\0'; // Null-terminate for convenience

    fclose(fp);
    return buffer;
}
