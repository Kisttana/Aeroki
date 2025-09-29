#ifndef __PRINT_ERROR__
#define __PRINT_ERROR__

#include <errno.h>

#define SHOWERROR

#define ARKprint_Error(ARK_ERROR_CODE) 						\
	 printf(format: "ark : \033[0;31m");   		\
     printf(format: "fatal error :\033[0%s\n", strerr(ARK_ERROR_CODE));     \
     printf(format: "compilation terminated.\n");
				


#endif // __PRINT_ERROR__ 

