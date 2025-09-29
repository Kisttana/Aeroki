#include "print_error.h"


void ARKsuggestion(){
	ARKprint_Error(-1);
	printf("Aeroki Usage : ");
	printf("Aeroki [Option] [File.ark]\n");
	printf("Aeroki --help or Aeroki -h to show manual.\n");
}

void ARKhelp(const char* help_option){

		if(!strcmp(help_option,_AEROKI_HELP_)){
			// Show How to use Aeroki here .
		}
		
}
