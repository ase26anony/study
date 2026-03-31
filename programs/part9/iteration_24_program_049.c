/* test-scalars.h - Testing TYPE_SCALAR */
#ifndef TEST_SCALARS_H
#define TEST_SCALARS_H

/* Basic scalar types with GTY markers */
typedef GTY(()) int my_int;
typedef GTY(()) char my_char;
typedef GTY(()) unsigned long my_ulong;
typedef GTY(()) double my_double;
typedef GTY(()) _Bool my_bool;

/* Scalar with qualifiers */
typedef GTY(()) const int const_int_t;
typedef GTY(()) volatile short volatile_short_t;

#endif /* TEST_SCALARS_H */
