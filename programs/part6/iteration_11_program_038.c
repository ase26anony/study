/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

/* TYPE_ARRAY: Array typedef */
typedef int int_array_t[10];
typedef char char_array_t[256];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *data, int value);
typedef void (*simple_callback_t)(void);

/* TYPE_STRING: String structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((skip)) data;
};

/* Non-annotated struct (may become TYPE_UNDEFINED) */
struct unmarked_struct {
    int x;
    double y;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) basic_struct {
    int id;
    char * GTY((skip)) name;
    double value;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    void * GTY((skip)) user_data;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct basic_struct * GTY((tag("0"))) base;
    struct gcc_string * GTY((tag("1"))) str;
    int_array_t numbers;
    enum color color;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_value;
    double double_value;
    char * GTY((skip)) string_value;
    void * GTY((skip)) ptr_value;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *basic_ptr_t;
typedef int *int_ptr_t;
typedef callback_func_t callback_ptr_t;

/* Linked list structure (for traversal) */
struct GTY(()) list_node {
    int data;
    struct list_node * GTY((tag("0"))) next;
    struct list_node * GTY((tag("1"))) prev;
};

/* Struct containing union */
struct GTY(()) union_container {
    int type;
    union data_union GTY((tag("type"))) value;
    callback_func_t GTY((skip)) handler;
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct basic_struct * GTY((tag("0"))) items[10];
    int count;
};

/* Mixed type container */
struct GTY(()) mixed_container {
    /* Scalar members */
    scalar_int_t id;
    scalar_double_t weight;
    
    /* Pointer members */
    struct complex_struct * GTY((tag("0"))) complex;
    int_ptr_t ints;
    
    /* Array member */
    char_array_t buffer;
    
    /* Union member */
    union data_union GTY((tag("1"))) data;
    
    /* Callback member */
    simple_callback_t GTY((skip)) on_event;
    
    /* String member */
    struct gcc_string * GTY((tag("2"))) description;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_type"), tag("lang_type"))) lang_struct {
    int lang_type;
    void * GTY((skip)) lang_data;
    struct GTY((skip)) unmarked_struct *unmarked;  /* Reference to non-GTY type */
};

/* Root structure containing everything */
struct GTY(()) root_container {
    struct basic_struct * GTY((tag("0"))) basic;
    struct complex_struct * GTY((tag("1"))) complex;
    struct user_struct * GTY((tag("2"))) user;
    struct list_node * GTY((tag("3"))) list_head;
    struct union_container * GTY((tag("4"))) union_cont;
    struct pointer_array * GTY((tag("5"))) ptr_array;
    struct mixed_container * GTY((tag("6"))) mixed;
    struct lang_struct * GTY((tag("7"))) lang;
    
    /* Direct members */
    union data_union root_data;
    struct gcc_string root_string;
    callback_func_t root_callback;
    
    /* Arrays */
    int scalar_array[5];
    struct basic_struct * GTY((tag("8"))) struct_array[3];
};

/* Additional pointer chain for deeper traversal */
struct GTY(()) deep_structure {
    struct deep_structure * GTY((tag("0"))) deeper;
    struct root_container * GTY((tag("1"))) root;
    int depth;
};

/* Function pointer with complex signature */
typedef struct root_container *(*factory_func_t)(
    int count, 
    struct basic_struct ** GTY((tag("0"))) items
);

/* Struct using the complex factory function */
struct GTY(()) factory_user {
    factory_func_t GTY((skip)) create;
    struct root_container * GTY((tag("0"))) product;
};

#endif /* GTY_TEST_H */
