/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* TYPE_ENUM (processed as scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((skip)) data;
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(void *data, int param);

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct GTY((user)) user_defined {
    int id;
    char *name;
};

/* TYPE_STRUCT: Regular structures */
struct GTY(()) base_struct {
    int id;
    scalar_double_t value;
    enum color color;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct base_struct * GTY((tag("0"))) base;
    struct gcc_string * GTY((skip)) str;
    callback_t handler;
    int flags;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char * GTY((skip)) string_val;
    void *ptr_val;
};

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10];
typedef struct base_struct *struct_ptr_array_t[5];

/* Array within a struct */
struct GTY(()) array_container {
    int_array_t numbers;
    struct_ptr_array_t pointers;
    char buffer[256];
};

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr_t;
typedef void (*void_func_ptr_t)(void);
typedef int_array_t *array_ptr_t;

/* Linked list structure (creates pointer chain) */
struct GTY(()) list_node {
    int value;
    struct list_node * GTY((tag("0"))) next;
    struct list_node * GTY((tag("1"))) prev;
};

/* Structure with all types combined */
struct GTY(()) type_kitchen_sink {
    /* Scalar members */
    int counter;
    double ratio;
    enum color current_color;
    
    /* Pointer members */
    struct base_struct * GTY((tag("0"))) base_ptr;
    struct complex_struct * GTY((tag("1"))) complex_ptr;
    void * GTY((skip)) opaque_ptr;
    
    /* Union member */
    union data_union current_data;
    
    /* Array members */
    int scores[5];
    struct base_struct * GTY((tag("2"))) ptr_array[3];
    
    /* String member */
    struct gcc_string description;
    
    /* Callback member */
    callback_t notify;
    
    /* Nested structure */
    struct {
        int x;
        int y;
    } position;
    
    /* Reference to user struct */
    struct user_defined * GTY((skip)) user_info;
    
    /* Linked list head */
    struct list_node * GTY((tag("3"))) list_head;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GENERATOR_FILE
struct GTY(()) lang_struct {
    int lang_specific_data;
    void * GTY((skip)) lang_pointer;
};
#endif

/* Root structure containing everything */
struct GTY(()) root_container {
    struct type_kitchen_sink * GTY((tag("0"))) sink;
    struct array_container * GTY((tag("1"))) arrays;
    union data_union * GTY((tag("2"))) unions[2];
    callback_t callbacks[3];
    struct list_node * GTY((tag("3"))) active_list;
    
    /* Direct scalar */
    scalar_int_t direct_scalar;
    
    /* Direct array */
    scalar_double_t direct_array[4];
};

/* Non-annotated types (may become TYPE_UNDEFINED) */
struct plain_struct {
    int a;
    int b;
};

typedef struct plain_struct *plain_ptr_t;

/* Forward declaration */
struct forward_declared;

/* Structure with forward declared pointer */
struct GTY(()) uses_forward {
    int id;
    struct forward_declared * GTY((skip)) fwd_ptr;
};

/* Actually define it later */
struct forward_declared {
    int value;
    struct uses_forward * GTY((tag("0"))) back_ref;
};

#endif /* GTY_TEST_H */
