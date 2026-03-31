/* Basic scalar types - TYPE_SCALAR */
#ifndef TEST_SCALARS_H
#define TEST_SCALARS_H

typedef int my_int;
typedef char my_char;
typedef long my_long;
typedef short my_short;
typedef unsigned int my_uint;
typedef float my_float;
typedef double my_double;

/* GTY-marked scalars */
typedef GTY(()) int gty_int;
typedef GTY(()) char gty_char;

#endif /* TEST_SCALARS_H */
