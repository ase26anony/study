/* scalar-types.h - GTY scalar type definitions */

#ifndef SCALAR_TYPES_H
#define SCALAR_TYPES_H

/* Basic scalar types */
typedef GTY(()) int my_scalar_t;
typedef GTY(()) unsigned int my_unsigned_scalar_t;
typedef GTY(()) long my_long_t;
typedef GTY(()) double my_double_t;
typedef GTY(()) char my_char_t;
typedef GTY(()) _Bool my_bool_t;

/* Enum types (treated as scalars) */
typedef GTY(()) enum { RED, GREEN, BLUE } color_t;
enum GTY(()) my_enum { VALUE1, VALUE2, VALUE3 };

/* Volatile and const qualified scalars */
typedef GTY(()) volatile int volatile_scalar_t;
typedef GTY(()) const long const_scalar_t;

#endif /* SCALAR_TYPES_H */
