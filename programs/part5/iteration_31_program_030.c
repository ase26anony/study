#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"
#include "system.h"
#include "coretypes.h"

/* ==================== UNDEFINED TYPES ==================== */
/* Forward declarations without definition - TYPE_UNDEFINED */
struct undefined_struct;
union undefined_union;

/* ==================== SCALAR TYPES ==================== */
/* Basic scalar types - TYPE_SCALAR */
typedef int GTY(()) int_scalar_t;
typedef float GTY(()) float_scalar_t;
typedef double GTY(()) double_scalar_t;
typedef char GTY(()) char_scalar_t;
typedef long GTY(()) long_scalar_t;
typedef short GTY(()) short_scalar_t;
typedef unsigned int GTY(()) uint_scalar_t;

/* ==================== STRING TYPE ==================== */
/* String pointer type - TYPE_STRING */
typedef const char * GTY(()) string_t;

/* ==================== POINTER TYPES ==================== */
/* Various pointer typedefs - TYPE_POINTER */
typedef int * GTY(()) int_ptr_t;
typedef float * GTY(()) float_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef void * GTY(()) void_ptr_t;

/* ==================== ARRAY TYPES ==================== */
/* Array typedefs - TYPE_ARRAY */
typedef int GTY(()) int_array_t[10];
typedef char GTY(()) char_array_t[256];
typedef struct simple_struct * GTY(()) struct_ptr_array_t[5];

/* ==================== CALLBACK TYPES ==================== */
/* Function pointer typedefs - TYPE_CALLBACK */
typedef void (*GTY(()) callback_t)(int);
typedef int (*GTY(()) compare_func_t)(const void *, const void *);
typedef void (*GTY(()) void_callback_t)(void);

/* ==================== SIMPLE STRUCTS ==================== */
/* Regular structs - TYPE_STRUCT */
struct GTY(()) simple_struct {
    int_scalar_t id;
    string_t name;
    int_array_t scores;
};

struct GTY(()) complex_member_struct {
    int_scalar_t count;
    float_scalar_t value;
    char_scalar_t flag;
    int_ptr_t data_ptr;
    callback_t handler;
};

/* ==================== USER STRUCTS ==================== */
/* User-defined structs - TYPE_USER_STRUCT */
typedef struct GTY(()) user_def_struct {
    int GTY(()) user_id;
    string_t GTY(()) user_name;
    callback_t GTY(()) user_callback;
} user_struct_t;

typedef struct GTY(()) nested_user_struct {
    user_struct_t GTY(()) user;
    int_array_t GTY(()) user_data;
    struct_ptr_t GTY(()) next;
} nested_user_t;

/* ==================== UNION TYPES ==================== */
/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int_scalar_t int_val;
    float_scalar_t float_val;
    string_t string_val;
    void_ptr_t ptr_val;
};

typedef union GTY(()) typed_union {
    int GTY(()) as_int;
    float GTY(()) as_float;
    char_array_t GTY(()) as_string;
} typed_union_t;

/* ==================== LANG STRUCTS ==================== */
/* Language-specific structs with GTY markers - TYPE_LANG_STRUCT */
struct GTY((user)) lang_struct_base {
    int GTY(()) lang_id;
    string_t GTY(()) lang_name;
};

struct GTY((desc("%0"))) lang_specific_struct {
    struct lang_struct_base GTY(()) base;
    union data_union GTY(()) data;
    callback_t GTY(()) lang_callback;
};

/* ==================== NESTED/COMPLEX TYPES ==================== */
/* Struct containing array of pointers to unions */
struct GTY(()) container_struct {
    int GTY(()) container_id;
    union data_union * GTY(()) union_ptrs[8];
    struct complex_member_struct GTY(()) member;
    typed_union_t GTY(()) variant;
};

/* Struct with function pointer array */
struct GTY(()) callback_container {
    callback_t GTY(()) handlers[4];
    compare_func_t GTY(()) comparator;
    struct container_struct * GTY(()) data;
};

/* ==================== EXTERNAL TYPES ==================== */
/* Include additional types from auxiliary header */
#include "test_types_aux.h"

/* ==================== TYPE USAGE EXAMPLES ==================== */
/* Global variables using our types (ensures they're processed) */
extern struct simple_struct GTY(()) global_simple;
extern user_struct_t GTY(()) global_user_struct;
extern union data_union GTY(()) global_union;
extern struct lang_specific_struct GTY(()) global_lang_struct;

#endif /* TEST_TYPES_H */
