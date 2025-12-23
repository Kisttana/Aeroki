#ifndef AEGIS_STRING
#define AEGIS_STRING

#include "aegis_vector.h"
#include "aegis_utils.h"

// Syntax => sget(<__string__>)[<index>]
#define sget(__string__) ((char*)(__string__)->data))

typedef vector* ag_string;
ag_string new_string(const char *cstr);
void string_append(ag_string  str,const char *cstr);

ag_string sub_string(const ag_string str, size_t pos, size_t len);

inline void string_ncopy(const ag_string  _src, const ag_string  _dest , size_t n);
inline void string_copy(const ag_string  _src, const ag_string  _dest);
inline ag_string string_rep(const ag_string str);
inline char * find_substr(const ag_string src_str, const ag_string str_to_search);

int sto_int(const ag_string str);
long long sto_ll(const ag_string str);
float sto_f(const ag_string str);
double sto_d(const ag_string str);
long double sto_ld(const ag_string str);

#endif //AEGIS_STRING
