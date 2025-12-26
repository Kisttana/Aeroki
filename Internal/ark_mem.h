#ifndef ARK_MEM_H
#define ARK_MEM_H

#include <string.h>
#include <stdlib.h>

#define ARK_MAXSIZE_OBJS 1024

#define ARK_ALLOCATE_MEM(__type__, __amount__ ) ((__type__*)malloc(sizeof(__type__) * __amount__))
#define NEW(__type__) ARK_ALLOCATE_MEM(__type__, 1)

#define ARK_DESTROY_ALLOCATED_MEM() 


//#define ARK_PUSH_GARBAGE(__MEM__)

#endif // ARK_MEM_H
