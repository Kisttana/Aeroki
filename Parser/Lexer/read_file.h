#ifndef __HANDLE_FILE__
#define __HANDLE_FILE__

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
char *read_file(FILE *fp);

#endif
