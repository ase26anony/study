#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration for undefined type - triggers TYPE_UNDEFINED */
struct undefined_struct;

/* Scalar type typedefs - triggers TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;

/* String type - triggers TYPE_STRING */
typedef const char * GTY(()) string_t;

/* Basic struct - triggers TYPE_STRUCT */
struct GTY(()) basic_struct {
    scalar_int_t field1;
    scalar_float_t field2;
    string_t field3;
};

/* Another struct with different composition */
struct GTY(()) complex_struct {
    scalar_double_t dbl_field;
    scalar_char_t char_field;
    struct basic_struct *nested;
};

/* User struct - triggers TYPE_USER_STRUCT */
typedef struct basic_struct GTY(()) user_struct_t;

/* Union type - triggers TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t int_val;
    scalar_float_t float_val;
    scalar_double_t double_val;
    string_t str_val;
};

/* Pointer types - triggers TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef struct basic_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* Array types - triggers TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct basic_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* Callback type (function pointer) - triggers TYPE_CALLBACK */
typedef void (* GTY(()) callback_t)(int, const char*);
typedef int (* GTY(()) another_callback_t)(struct basic_struct*, union data_union*);

/* Language-specific struct with GTY markers - triggers TYPE_LANG_STRUCT */
struct GTY((user)) lang_specific_struct {
    int GTY((skip)) ignored_field;  /* Skip this for GC */
    void * GTY((desc("%1"))) tagged_ptr;
    callback_t handler;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains multiple type categories */
    scalar_int_t scalar_member;          /* TYPE_SCALAR */
    string_t string_member;              /* TYPE_STRING */
    struct basic_struct struct_member;   /* TYPE_STRUCT */
    union data_union union_member;       /* TYPE_UNION */
    int_ptr_t pointer_member;            /* TYPE_POINTER */
    int_array_t array_member;            /* TYPE_ARRAY */
    callback_t callback_member;          /* TYPE_CALLBACK */
    struct lang_specific_struct* lang_struct_ptr; /* TYPE_LANG_STRUCT */
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
