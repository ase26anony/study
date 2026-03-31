#ifndef TEST_SCALARS_H
#define TEST_SCALARS_H

/* TYPE_SCALAR: Basic scalar types */
typedef GTY(()) int my_int;
typedef GTY(()) char my_char;
typedef GTY(()) unsigned long my_ulong;
typedef GTY(()) double my_double;

/* TYPE_STRING: String pointer types */
typedef GTY(()) const char *my_string;
typedef GTY(()) char *mutable_string;

#endif /* TEST_SCALARS_H */
