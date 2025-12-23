#ifndef AEGIS_UTILS
#define AEGIS_UTILS




#define agmax(a,b) \
     ({                             \
          __typeof__ (a) _a = (a);  \
          __typeof__ (b) _b = (b);  \
          _a > _b ? _a : _b;        \
     })

#define agmin(a,b) \
     ({                            \
          __typeof__ (a) _a = (a); \
          __typeof__ (b) _b = (b); \
          _a > _b ? _b :_a;         \
     })


#endif // AEGIS_UTILS
