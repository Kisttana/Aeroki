#ifndef AEGIS_VECTOR_H
#define AEGIS_VECTOR_H

#include "aegis_common.h"
typedef struct __VECTOR__{
     // Public :
     void* data;         // Pointer to the raw memory buffer


     // private read-only
     size_t size;        // Number of elements currently stored
     size_t capacity;    // Total slots available before realloc needed
     size_t element_size;// Size of a single element in bytes (e.g., 4 for int)
} vector;



// User API .

#define vec_wrap_val(type, val) ((type[]){val})

#define vec_init(__size__, type, set_zero) \ 
     __vec_init(__size__, sizeof(type), set_zero  )

#define vec_init_fill(__size__ , type , val) \ 
     __vec_init_fill(__size__, sizeof(type), vec_wrap_val(type, val) )

#define vec_fill(__ptrvec__, begin, end, type ,val) \ 
     __vec_fill(__ptrvec__, begin, end, vec_wrap_val(type, val))

#define vec_at_ptr(__ptrvec__ ,type, index) (((type*)((__ptrvec__)->data)) + index)
#define vec_at(__ptrvec__, type, index) *(vec_at_ptr(__ptrvec__, type, index))


#define vec_push_back(__ptrvec__, type, val) \
          __vec_push_back(__ptrvec__, vec_wrap_val(type, val) )

#define vec_pop_back(__ptrvec__) __vec_pop_back(__ptrvec__) 

#define vec_shrink_to_fit(__ptrvec__) __vec_shrink_to_fit(__ptrvec__);

#define vec_insert(__ptrvec__, __it__, type, val) \
          __vec_insert(__ptrvec__, __it__, vec_wrap_val(type,val))

#define vec_erase(__ptrvec__, __it__) \
          __vec_erase(__ptrvec__, __it__)

#define vec_clean(__ptrvec__) \
          __vec_clean(__ptrvec__)
// Core Functions
vector *__vec_init(size_t init_size,size_t element_size, bool set_zero);
vector *__vec_init_fill(size_t init_size, size_t element_size,void *val);

void __vec_fill(vector* vec, size_t begin, size_t end,void *val);

void __vec_push_back(vector* vec, void* element);
void __vec_pop_back(vector* vec);
void __vec_shrink_to_fit(vector *vec);
void __vec_clean(vector* vec);
void __vec_insert(vector* vec, size_t pos,void *val);
void __vec_erase(vector* vec, size_t pos);

void __vec_set_capacity(vector* vec,size_t new_capacity);

#endif //AEGIS_VECTOR_H
