#ifndef __HANDLE_FILE__
#define __HANDLE_FILE__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <error.h>


/*
 * @breif Read provieded file into string.
 *
 * @param file_path is provieded file name
 * @return string of provieded file contents.
*/
char *read_file(const char *file_path);

#endif
