/* Basic scalar types */
#ifndef TEST_SCALARS_H
#define TEST_SCALARS_H

typedef GTY(()) int my_int;
typedef GTY(()) char my_char;
typedef GTY(()) long my_long;
typedef GTY(()) short my_short;
typedef GTY(()) unsigned int my_uint;

/* String types */
typedef GTY(()) const char *my_string;
typedef GTY(()) char *mutable_string;

#endif /* TEST_SCALARS_H */
