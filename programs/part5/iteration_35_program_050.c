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

/* Scalar with qualifiers */
typedef GTY(()) const int my_const_scalar_t;
typedef GTY(()) volatile long my_volatile_scalar_t;

/* Enum type (treated as scalar) */
typedef GTY(()) enum my_enum { ENUM_A, ENUM_B, ENUM_C } my_enum_t;

#endif /* SCALAR_TYPES_H */
