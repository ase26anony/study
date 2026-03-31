#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include necessary GCC headers for GTY macros */
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
struct GTY(()) simple_struct {
    scalar_int_t id;
    string_t name;
};

struct GTY(()) complex_struct {
    scalar_int_t count;
    scalar_float_t values[5];
    struct simple_struct * GTY((skip)) next;
};

/* TYPE_USER_STRUCT: Struct with user-defined type */
typedef struct simple_struct user_struct_t;
struct GTY(()) container_struct {
    user_struct_t GTY((tag("0"))) item;
    scalar_int_t index;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_t as_string;
    void * GTY((skip)) as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef int * GTY(()) int_ptr_t;
typedef struct simple_struct * GTY(()) struct_ptr_t;
typedef union data_union * GTY(()) union_ptr_t;

/* TYPE_ARRAY: Array typedefs */
typedef int GTY(()) int_array_t[10];
typedef struct simple_struct GTY(()) struct_array_t[5];
typedef union data_union GTY(()) union_array_t[3];

/* TYPE_CALLBACK: Function pointer typedefs */
typedef void (* GTY(()) callback_t)(int);
typedef int (* GTY(()) filter_t)(const char *, void *);

/* TYPE_LANG_STRUCT: Language-specific struct with GTY markers */
struct GTY((user)) lang_struct_base {
    int GTY((skip)) lang_specific;
    void * GTY((desc("%1"))) lang_data;
};

/* Complex nested type to ensure traversal hits all categories */
struct GTY(()) master_struct {
    /* Scalar */
    scalar_int_t id;
    
    /* String */
    string_t description;
    
    /* Struct */
    struct simple_struct basic;
    
    /* User struct */
    user_struct_t user_data;
    
    /* Union */
    union data_union variant;
    
    /* Pointer */
    int_ptr_t numbers;
    struct_ptr_t next_struct;
    
    /* Array */
    int_array_t scores;
    struct_array_t items;
    
    /* Array of pointers */
    union_ptr_t GTY((length("%0.union_count"))) union_ptrs[5];
    
    /* Callback */
    callback_t notify;
    
    /* Language struct */
    struct lang_struct_base * GTY((skip)) lang_info;
    
    /* For array length */
    int union_count;
};

/* Include secondary header for additional types */
#include "test_types_aux.h"

#endif /* TEST_TYPES_H */
