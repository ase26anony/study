#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
#include "gtype-desc.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* TYPE_SCALAR: Basic scalar types with GTY annotations */
typedef int GTY(()) scalar_int_t;
typedef float GTY(()) scalar_float_t;
typedef double GTY(()) scalar_double_t;
typedef char GTY(()) scalar_char_t;

/* TYPE_STRING: String pointer typedef */
typedef const char * GTY(()) string_t;

/* TYPE_STRUCT: Various struct types */
struct GTY(()) simple_struct {
    scalar_int_t field1;
    scalar_float_t field2;
};

struct GTY(()) complex_struct {
    scalar_int_t id;
    string_t name;
    struct simple_struct * GTY(()) nested;
};

/* TYPE_USER_STRUCT: Struct with user-defined marker */
struct GTY((user)) user_struct {
    int user_data;
    void * GTY((skip)) user_ptr;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef string_t GTY(()) string_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) filter_t)(const char *, int);

/* TYPE_LANG_STRUCT: Language-specific struct with GC roots */
struct GTY((desc("%1"), chain_next("%0.next"), chain_prev("%0.prev"))) lang_struct {
    int lang_specific;
    struct lang_struct * GTY(()) next;
    struct lang_struct * GTY(()) prev;
};

/* Complex nested type to ensure traversal */
struct GTY(()) container_struct {
    /* Contains scalar */
    scalar_int_t count;
    
    /* Contains string */
    string_t description;
    
    /* Contains struct */
    struct simple_struct simple;
    
    /* Contains user struct */
    struct user_struct user;
    
    /* Contains union */
    union data_union data;
    
    /* Contains pointer */
    int_ptr_t ints;
    
    /* Contains array */
    int_array_t numbers;
    
    /* Contains callback */
    callback_t handler;
    
    /* Contains language struct */
    struct lang_struct * GTY(()) lang_items;
    
    /* Nested pointer to undefined struct */
    struct undefined_struct * GTY(()) undefined_ptr;
};

/* Include auxiliary header for more types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
