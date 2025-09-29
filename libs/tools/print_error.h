#ifndef __PRINT_ERROR__
#define __PRINT_ERROR__

#include <errno.h>
#include <string.h>
#include <stdio.h>

#define SHOWERROR

#define _AEROKI_HELP_ "HELP"




#define ARKprint_Error(ARK_ERROR_CODE) 						\
	 printf("ark : \033[0;31m");   		\
     printf("fatal error :\033[0;m %s\n", strerror(ARK_ERROR_CODE));     \
     printf("compilation terminated.\n");
				
void ARKsuggestion();
void ARKhelp(const char* help_option);
void ARKusage();
void ARKcomplie();
void ARKllvm();
void ARKlist_option();

#endif // __PRINT_ERROR__ 

