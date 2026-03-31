#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's type annotation macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR: Basic scalar types */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* TYPE_STRING: String pointer type */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) regular_struct {
    scalar_int_t field1;
    scalar_float_t field2;
    string_t field3;
};

/* Another struct with different composition */
struct GTY(()) another_struct {
    scalar_double_t dbl_field;
    scalar_char_t char_field;
    struct regular_struct *nested_struct;
};

/* TYPE_USER_STRUCT: Struct with user-defined alignment/size */
struct GTY((user)) user_struct {
    int GTY((skip)) skipped_field;  /* Field not traced by GC */
    void * GTY((tag("0"))) tagged_ptr;
    long custom_aligned_field GTY((aligned(16)));
};

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void *as_pointer;
};

/* TYPE_POINTER: Pointer typedefs */
typedef int * GTY(()) int_ptr_t;
typedef struct regular_struct * GTY(()) struct_ptr_t;
typedef union test_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int GTY(()) int_array_t[10];
typedef struct regular_struct GTY(()) struct_array_t[5];
typedef union test_union GTY(()) union_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) process_callback_t)(const char *, void *);
typedef void (* GTY(()) complex_callback_t)(struct regular_struct *, union test_union *);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((desc("%1.lang_code"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int lang_code;
    string_t lang_name;
    callback_t lang_handler;
    struct lang_struct * GTY((skip)) next;
    struct lang_struct *prev;
};

/* Complex nested type to ensure traversal of all categories */
struct GTY(()) complex_nested {
    /* Contains scalar */
    scalar_int_t count;
    
    /* Contains string */
    string_t name;
    
    /* Contains struct */
    struct regular_struct nested_struct;
    
    /* Contains union */
    union test_union data_union;
    
    /* Contains pointer */
    int_ptr_t dynamic_data;
    
    /* Contains array */
    int_array_t fixed_data;
    
    /* Contains callback */
    callback_t notify;
    
    /* Contains pointer to lang_struct */
    struct lang_struct * GTY((tag("1"))) lang_info;
    
    /* Contains array of pointers */
    struct_ptr_t GTY((length("%0.count"))) *struct_ptrs;
};

/* Include auxiliary types from another header */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
