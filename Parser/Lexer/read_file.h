#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


/*
 * @breif Read provieded file into string.
 *
 * @param fp file to read
 * @return string of provieded file contents.
*/
char *read_file(const char *file_path);

#endif //READ_FILE_H
