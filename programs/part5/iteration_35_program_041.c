/* scalar-types.h - Basic scalar and string type definitions */

#ifndef SCALAR_TYPES_H
#define SCALAR_TYPES_H

/* TYPE_SCALAR cases */
typedef GTY(()) int my_scalar_t;
typedef GTY(()) unsigned int my_unsigned_scalar_t;
typedef GTY(()) long my_long_t;
typedef GTY(()) double my_double_t;
typedef GTY(()) char my_char_t;
typedef GTY(()) _Bool my_bool_t;

/* TYPE_STRING cases */
typedef GTY(()) const char * my_string_t;
typedef GTY(()) char * my_mutable_string_t;
typedef GTY(()) const char * const my_const_string_ptr_t;

/* Macro-generated scalar variants */
#define DEF_SCALAR_TYPE(name, type) typedef GTY(()) type name##_t
DEF_SCALAR_TYPE(macro_int, int);
DEF_SCALAR_TYPE(macro_float, float);
DEF_SCALAR_TYPE(macro_size, size_t);

#endif /* SCALAR_TYPES_H */
