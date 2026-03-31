#ifndef TEST_SCALARS_H
#define TEST_SCALARS_H

/* TYPE_SCALAR examples */
typedef GTY(()) int my_int;
typedef GTY(()) char my_char;
typedef GTY(()) unsigned long my_ulong;
typedef GTY(()) double my_double;
typedef GTY(()) _Bool my_bool;

/* TYPE_STRING examples */
typedef GTY(()) const char *my_string;
typedef GTY(()) char *mutable_string;
typedef GTY(()) const char * const constant_string_ptr;

#endif /* TEST_SCALARS_H */
