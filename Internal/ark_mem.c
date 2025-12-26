#include "ark_mem.h"
#include <stdio.h>
#include <stdlib.h>

extern void *Ark_memleak_objs[ARK_MAXSIZE_OBJS];
static size_t Ark_memleak_currentIndex = 0;
void __Ark_Add_MemLeak(void *__memleak__){
     if (__memleak__ == NULL) {
          fprintf(stderr, "[ERROR] : Adding NULL Pointer into Leaked Memory list");
     }
     Ark_memleak_objs[Ark_memleak_currentIndex++]  = __memleak__;

}

void __Ark_Destroy_MemLeak(){

}
