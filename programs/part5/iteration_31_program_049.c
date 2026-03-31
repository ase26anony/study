#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* ==================== SCALAR TYPES ==================== */
/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;
typedef long GTY(()) scalar_long_t;

/* ==================== STRING TYPE ==================== */
/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* ==================== UNDEFINED TYPES ==================== */
/* TYPE_UNDEFINED: Forward declarations without definition */
struct GTY(()) undefined_struct;
union GTY(()) undefined_union;

/* ==================== STRUCT TYPES ==================== */
/* TYPE_STRUCT: Regular struct types */

/* Simple struct with scalar members */
struct GTY(()) simple_struct {
    scalar_int_t id;
    scalar_float_t value;
    string_t name;
};

/* Nested struct containing other structs */
struct GTY(()) complex_struct {
    struct simple_struct base;
    struct simple_struct * GTY((skip)) next;  /* Skip for traversal */
    int GTY(()) count;
};

/* Another struct type for variety */
struct GTY(()) another_struct {
    double GTY(()) data;
    char GTY(()) flag;
};

/* ==================== USER STRUCT TYPES ==================== */
/* TYPE_USER_STRUCT: User-defined struct types */
typedef struct simple_struct GTY(()) user_struct_t;
typedef struct complex_struct GTY(()) complex_user_struct_t;

/* ==================== UNION TYPES ==================== */
/* TYPE_UNION: Union types */

/* Simple union */
union GTY(()) data_union {
    scalar_int_t int_val;
    scalar_float_t float_val;
    string_t string_val;
    void * GTY((skip)) ptr_val;
};

/* Union containing structs */
union GTY(()) struct_union {
    struct simple_struct simple;
    struct complex_struct complex;
};

/* ==================== POINTER TYPES ==================== */
/* TYPE_POINTER: Pointer typedefs */

typedef scalar_int_t * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;
typedef void (* GTY(()) void_func_ptr_t)(void);

/* Pointer to pointer */
typedef int_ptr_t * GTY(()) int_ptr_ptr_t;

/* ==================== ARRAY TYPES ==================== */
/* TYPE_ARRAY: Array typedefs */

typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];
typedef string_t GTY(()) string_array_t[8];

/* Multi-dimensional array */
typedef int GTY(()) matrix_t[4][4];

/* ==================== CALLBACK TYPES ==================== */
/* TYPE_CALLBACK: Function pointer typedefs */

typedef void (* GTY(()) simple_callback_t)(int);
typedef int (* GTY(()) process_callback_t)(struct simple_struct *, string_t);
typedef void (* GTY(()) complex_callback_t)(int_array_t, struct_ptr_t);

/* Callback that returns a callback */
typedef simple_callback_t (* GTY(())) callback_factory_t)(int);

/* ==================== LANGUAGE STRUCT TYPES ==================== */
/* TYPE_LANG_STRUCT: GCC language-specific structs with GTY markers */

/* Tree-related structure (common in GCC) */
struct GTY(()) tree_common {
    int GTY(()) code;
    union tree_common * GTY((skip)) chain;
};

/* Language-specific structure for frontend */
struct GTY(()) lang_type {
    struct tree_common common;
    unsigned int GTY(()) lang_flag;
    void * GTY((skip)) lang_data;
};

/* Another language struct */
struct GTY(()) lang_decl {
    int GTY(()) decl_context;
    struct lang_type * GTY((skip)) associated_type;
};

/* ==================== COMPLEX NESTED TYPES ==================== */
/* To ensure deep traversal of type graph */

struct GTY(()) container_struct {
    /* Contains array of pointers to unions */
    union data_union * GTY(()) union_ptrs[5];
    
    /* Contains struct with callback */
    struct GTY(()) nested_with_callback {
        int GTY(()) id;
        simple_callback_t GTY(()) handler;
    } callback_struct;
    
    /* Pointer to array of structs */
    struct simple_struct (* GTY(()) struct_array_ptr)[3];
    
    /* Union containing different types */
    union GTY(()) nested_union {
        int GTY(()) int_val;
        struct nested_with_callback GTY(()) callback_val;
        int_array_t GTY(()) array_val;
    } data;
};

/* ==================== FORWARD DECLARATIONS ==================== */
/* More undefined types for TYPE_UNDEFINED count */
struct GTY(()) forward_declared_struct;
typedef struct forward_declared_struct GTY(()) forward_struct_t;

union GTY(()) forward_declared_union;
typedef union forward_declared_union GTY(()) forward_union_t;

/* ==================== INCLUDE AUXILIARY TYPES ==================== */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
