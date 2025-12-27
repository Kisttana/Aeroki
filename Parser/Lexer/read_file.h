/**
 * @file read_file.h
 * @author Piyaphat Jaiboon (fiw.contact.work@gmail.com) 
 * @brief  provided reaf_file function which use in Lexer implementation. 
 * @version 0.1
 * @date 2025-12-28
 * 
 * @copyright Copyright (c) 2025
 * 
 */
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
