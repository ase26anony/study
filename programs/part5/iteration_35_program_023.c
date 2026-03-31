#ifndef SCALAR_TYPES_H
#define SCALAR_TYPES_H

/* TYPE_SCALAR */
typedef GTY(()) int my_scalar_t;
typedef GTY(()) unsigned long my_ulong_t;
typedef GTY(()) double my_double_t;
typedef GTY(()) char my_char_t;
typedef GTY(()) _Bool my_bool_t;

/* TYPE_STRING */
typedef GTY(()) const char * my_string_t;
typedef GTY(()) char * mutable_string_t;
typedef GTY(()) const char * const const_string_ptr_t;

/* Macro-generated scalar variants */
#define DEF_SCALAR_TYPE(name, type) typedef GTY(()) type name##_t
DEF_SCALAR_TYPE(int8, signed char);
DEF_SCALAR_TYPE(uint8, unsigned char);
DEF_SCALAR_TYPE(int16, short);
DEF_SCALAR_TYPE(uint16, unsigned short);
DEF_SCALAR_TYPE(int32, int);
DEF_SCALAR_TYPE(uint32, unsigned int);
DEF_SCALAR_TYPE(int64, long long);
DEF_SCALAR_TYPE(uint64, unsigned long long);

#endif /* SCALAR_TYPES_H */
