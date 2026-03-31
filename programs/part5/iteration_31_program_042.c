#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* 1. SCALAR TYPES (TYPE_SCALAR) */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* 2. STRING TYPE (TYPE_STRING) */
typedef const char *GTY(()) string_t;

/* 3. UNDEFINED TYPE (TYPE_UNDEFINED) - forward declaration */
struct GTY(()) undefined_struct;
typedef struct undefined_struct *GTY(()) undefined_ptr_t;

/* 4. STRUCT TYPES (TYPE_STRUCT) */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct *GTY(()) nested;
};

/* 5. USER STRUCT (TYPE_USER_STRUCT) */
/* User structs are typically defined with GTY((user)) */
struct GTY((user)) user_defined_struct {
    int custom_field;
    void *GTY((skip)) opaque_data;
};

/* 6. UNION TYPE (TYPE_UNION) */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* 7. POINTER TYPES (TYPE_POINTER) */
typedef int *GTY(()) int_ptr_t;
typedef struct simple_struct *GTY(()) struct_ptr_t;
typedef union data_union *GTY(()) union_ptr_t;

/* 8. ARRAY TYPES (TYPE_ARRAY) */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* 9. CALLBACK TYPE (TYPE_CALLBACK) */
typedef void (*GTY(()) callback_t)(int, const char*);
typedef int (*GTY(()) filter_func_t)(struct simple_struct *);

/* 10. LANGUAGE STRUCT (TYPE_LANG_STRUCT) */
/* Use GTY with special markers for language-specific structures */
struct GTY((desc("%1.lang_code"), tag("LANG_STRUCT"))) lang_specific_struct {
    int lang_code;
    void *GTY((length("strlen(%h.data)+1"))) data;
    callback_t handler;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container_struct {
    int_array_t numbers;
    struct_array_t objects;
    union_array_t variants;
    int_ptr_t *GTY(()) pointer_array; /* Array of pointers */
    callback_t handlers[4];
    struct lang_specific_struct *GTY(()) lang_struct;
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
