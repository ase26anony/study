#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration for TYPE_UNDEFINED */
struct undefined_struct;

/* Scalar types (TYPE_SCALAR) */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;
typedef unsigned int GTY(()) scalar_uint_t;

/* String type (TYPE_STRING) */
typedef const char *GTY(()) string_t;

/* Pointer types (TYPE_POINTER) */
typedef int *GTY(()) int_ptr_t;
typedef float *GTY(()) float_ptr_t;
typedef void *GTY(()) void_ptr_t;
typedef struct defined_struct *GTY(()) struct_ptr_t;

/* Array types (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef char GTY(()) char_array_t[256];
typedef float GTY(()) float_array_t[5][5];

/* Callback types (TYPE_CALLBACK) */
typedef void (*GTY(()) callback_t)(int);
typedef int (*GTY(()) callback_with_return_t)(const char *, int);
typedef void (*GTY(()) complex_callback_t)(int, float, void *);

/* Simple struct (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    int field1;
    float field2;
    char field3;
};

/* More complex struct with nested types */
struct GTY(()) complex_struct {
    scalar_int_t int_field;
    string_t string_field;
    int_ptr_t pointer_field;
    int_array_t array_field;
    callback_t callback_field;
    struct simple_struct nested_struct;
};

/* Union type (TYPE_UNION) */
union GTY(()) test_union {
    int int_value;
    float float_value;
    char *string_value;
    void *pointer_value;
};

/* Struct containing union */
struct GTY(()) struct_with_union {
    int type_tag;
    union test_union data;
};

/* Array of pointers */
typedef struct simple_struct *GTY(()) struct_ptr_array_t[5];

/* Pointer to array */
typedef int (*GTY(()) array_ptr_t)[10];

/* User struct (TYPE_USER_STRUCT) - typically via typedef */
typedef struct GTY(()) user_def_struct {
    int id;
    const char *name;
    float values[3];
} user_struct_t;

/* Language-specific struct (TYPE_LANG_STRUCT) with GCC-specific annotations */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) lang_struct {
    int lang_specific_field;
    void *lang_data;
    struct lang_struct *GTY((skip)) next;
    struct lang_struct *GTY((skip)) prev;
};

/* Another language struct with different annotations */
struct GTY((desc("%1.type"), tag("TYPE_ENUM"))) another_lang_struct {
    int type;
    union {
        int int_val;
        float float_val;
        const char *string_val;
    } GTY((desc("%0.type"))) data;
};

/* Struct with callback field */
struct GTY(()) struct_with_callback {
    int id;
    callback_t handler;
    void *user_data;
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
