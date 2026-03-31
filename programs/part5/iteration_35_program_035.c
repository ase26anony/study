/* scalar-types.h - GTY scalar type definitions */

#ifndef SCALAR_TYPES_H
#define SCALAR_TYPES_H

/* Basic scalar types */
typedef GTY(()) int my_scalar_t;
typedef GTY(()) unsigned int my_unsigned_scalar_t;
typedef GTY(()) long my_long_scalar_t;
typedef GTY(()) double my_double_scalar_t;
typedef GTY(()) _Bool my_bool_scalar_t;

/* Enum types (treated as scalars) */
typedef enum GTY(()) my_enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
} my_enum_t;

/* Macro-generated scalar variants */
#define DEF_SCALAR_TYPE(name, type) typedef GTY(()) type name##_t
DEF_SCALAR_TYPE(macro_int, int);
DEF_SCALAR_TYPE(macro_char, char);
DEF_SCALAR_TYPE(macro_short, short);

#endif /* SCALAR_TYPES_H */
