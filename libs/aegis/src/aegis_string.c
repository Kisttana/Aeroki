#include "../include/aegis/aegis_string.h"
#include <string.h>

ag_string 
new_string(const char *cstr){
     vector *buffer = vec_init(strlen(cstr), char, 1);
     strcpy((char*)buffer->data, cstr); 
     return buffer;
}

void string_append(ag_string  str,const char *cstr){
     size_t nsize = str->size + strlen(cstr);
     if( nsize >= str->capacity) 
          __vec_set_capacity(str, (size_t)(nsize * 1.5 )) ;
     char *tail = ((char*)str->data) + str->size ;
     strcpy(tail, cstr);

     str->size = nsize;
     vec_at(str, char, nsize) = '\n';
}
ag_string 
sub_string(const ag_string str, size_t pos, size_t len){
     if (pos >= str->size) return NULL;

     if(pos + len > str->size) len = str->size - len;
     
     char substring[len+1] = {'\n'};

     strncpy(substring,((char *)str->data) + pos,  len);

     return new_string(substring); 
}

inline ag_string 
string_rep(const ag_string str) {
     return sub_string(str, 0, str->size); 
}

inline void 
string_ncopy(const ag_string _dest, const ag_string _src, size_t n)
{
     strncpy(_dest->data, _src->data, agmax(n, _dest->size));
}

inline void
string_copy(const ag_string _dest ,const ag_string _src)
{
     strcpy(_dest->data, _src->data);
}

inline char*
find_substr(const ag_string src_str, const ag_string str_to_search)
{
     return (!src_str->data && !str_to_search->data) ? NULL : strstr( (char*)src_str->data, (char*) str_to_search->data);
}

int main(){}
