#ifndef TEST_SCALARS_H
#define TEST_SCALARS_H

/* TYPE_SCALAR examples */
typedef GTY(()) int my_int;
typedef GTY(()) char my_char;
typedef GTY(()) unsigned long my_ulong;
typedef GTY(()) float my_float;
typedef GTY(()) double my_double;

/* Scalar with qualifiers */
typedef GTY(()) const int const_int_t;
typedef GTY(()) volatile short volatile_short_t;

#endif /* TEST_SCALARS_H */
