#include "aegis/aegis_utils.h"
#include "aegis/aegis_common.h"
#include "aegis/aegis_vector.h"



#define VEC_DEFLUAT_CAPACITY 8
#define VEC_NEW_CAPACITY(CURR_SIZE) ((size_t)(CURR_SIZE * 1.5))
#define VEC_MUST_INCREASE_CAPACITY(__ptrvec__) \
        if((__ptrvec__)->size == (__ptrvec__)->capacity) \
               __vec_set_capacity(__ptrvec__ , VEC_NEW_CAPACITY((__ptrvec__)->capacity))


vector*
__vec_init(size_t init_size,size_t element_size , bool set_zero)
{
     vector *vec = (vector*)malloc(sizeof(vector));
     
     assert(vec != NULL);

     size_t ncap = agmax(init_size, VEC_DEFLUAT_CAPACITY);
     vec->size = init_size;
     vec->capacity = ncap;
     vec->element_size = element_size; 
     
     // acllocating memory
     vec->data = (set_zero) ? calloc(vec->capacity, element_size) : malloc(element_size * ncap);

     // check if data has been sussessfully acllocated
     if(vec->data == NULL) {
          fprintf(stdin, "data wasn't sussessfully acllocated");
          free(vec);
          exit(1);
     }
     return vec;
}


vector* __vec_init_fill(size_t init_size,size_t element_size, void *val)
{
     vector *vec = __vec_init(init_size, element_size, 0);
     __vec_fill(vec, 0, init_size, val);
     
     return vec;
}

void 
__vec_fill(vector* vec, size_t begin, size_t end,void *val)
{
     size_t it = begin;

     // prevent iterate over size;
     size_t itend = agmin(vec->size,end);
     
     byte *idx = (byte*)vec->data;
     for( ;it < itend; it++)
          memcpy(idx + (it * vec->element_size), val, vec->element_size);
}

void 
__vec_push_back(vector* vec, void *val)
{
     VEC_MUST_INCREASE_CAPACITY(vec);

     byte *begin = (byte*)vec->data;
     memcpy(begin + (vec->size * vec->element_size), val, vec->element_size);
     vec->size++;
}

  void 
__vec_pop_back(vector *vec)
{
     if(vec->size > 0) vec->size--;
}

void 
__vec_set_capacity(vector *vec, size_t new_capacity)
{
     vec->capacity = new_capacity;
     void *temp = realloc(vec->data, new_capacity * vec->element_size); 
     assert(temp != NULL);
     vec->data = temp;
}

  void  
__vec_shrink_to_fit(vector *vec)
{
     __vec_set_capacity(vec, vec->size);
}

void 
__vec_insert(vector* vec, size_t pos, void *val)
{
     assert(pos < vec->size);

     VEC_MUST_INCREASE_CAPACITY(vec);
     
     size_t num_element_to_shift = vec->size - pos;
     size_t nsize = vec->element_size;
     byte *begin = (byte*)vec->data;
     
     // Shift each elements to right 1 slot
     memmove(begin + ((pos + 1) * nsize), 
            begin + (pos * nsize), 
            num_element_to_shift * nsize);
     
     // copy val to vector[position index] 
     memcpy(begin + (pos * nsize), 
            val, 
            vec->element_size);

     vec->size++;
}
void 
__vec_erase(vector* vec, size_t pos)
{
     assert(vec->size >= 1);
     
     size_t num_element_to_shift = vec->size - pos - 1;
     size_t nsize = vec->element_size;
     byte *begin = (byte*)vec->data;
     memmove(begin + (pos * nsize), 
            begin + ((pos + 1) * nsize), 
            num_element_to_shift * nsize);
      
     vec->size--;
}
void 
__vec_clean(vector* vec)
{
     free(vec->data);
     free(vec);
     vec = NULL;
}
