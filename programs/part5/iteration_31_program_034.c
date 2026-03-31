#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* Forward declaration - will be TYPE_UNDEFINED */
struct GTY(()) undefined_struct;

/* Scalar types - TYPE_SCALAR */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;

/* String type - TYPE_STRING */
typedef const char *GTY(()) string_t;

/* Struct types - TYPE_STRUCT */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct *GTY(()) nested;
};

/* User struct - TYPE_USER_STRUCT */
/* This typically comes from typedef struct {...} name_t */
typedef struct GTY(()) {
    scalar_int_t x;
    scalar_float_t y;
} user_struct_t;

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* Pointer types - TYPE_POINTER */
typedef int *GTY(()) int_ptr_t;
typedef struct simple_struct *GTY(()) struct_ptr_t;
typedef union data_union *GTY(()) union_ptr_t;

/* Array types - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/* Callback types - TYPE_CALLBACK */
typedef void (*GTY(()) callback_t)(int, const char*);
typedef int (*GTY(()) filter_func_t)(const void *);

/* Language-specific struct - TYPE_LANG_STRUCT */
/* These are typically marked with special GTY options */
struct GTY((user)) lang_specific_struct {
    int lang_specific_field;
    void *GTY((skip)) opaque_data;
};

/* Nested complex type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains various type kinds */
    scalar_int_t count;                     /* TYPE_SCALAR */
    string_t description;                   /* TYPE_STRING */
    struct complex_struct data;             /* TYPE_STRUCT */
    union data_union variant;               /* TYPE_UNION */
    int_ptr_t numbers;                      /* TYPE_POINTER */
    int_array_t buffer;                     /* TYPE_ARRAY */
    callback_t handler;                     /* TYPE_CALLBACK */
    struct lang_specific_struct *lang_data; /* TYPE_LANG_STRUCT */
    struct undefined_struct *future;        /* TYPE_UNDEFINED */
};

/* Another user struct with function pointer */
typedef struct GTY(()) {
    string_t name;
    callback_t notify;
    user_struct_t data;
} event_handler_t;

/* Include auxiliary types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
