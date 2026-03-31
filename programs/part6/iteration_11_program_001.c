/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef double scalar_double_t;

/* Enum type (also scalar) */
enum color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_UNDEFINED: Non-GTY type that will be referenced */
struct undefined_struct {
    int x;
    float y;
};

/* TYPE_STRUCT: Basic struct with GTY marker */
struct GTY(()) base_struct {
    int id;
    char name[32];  /* TYPE_ARRAY within struct */
    double value;
};

/* Another struct with nested references */
struct GTY(()) complex_struct {
    struct base_struct *base_ptr;  /* TYPE_POINTER */
    int scores[10];                /* TYPE_ARRAY */
    enum color color;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;  /* TYPE_POINTER to char */
    void *ptr_val;     /* TYPE_POINTER to void */
};

/* TYPE_ARRAY: Typedef for array type */
typedef int int_array_t[100];
typedef struct base_struct* struct_ptr_array_t[50];

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char *data;  /* TYPE_POINTER to char */
};

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *context, int value);

/* Struct using callback */
struct GTY(()) callback_container {
    callback_func_t handler;  /* TYPE_CALLBACK */
    void *context;            /* TYPE_POINTER */
    int threshold;
};

/* Linked list structure for chained references */
struct GTY(()) list_node {
    int data;
    struct list_node *next;  /* TYPE_POINTER to self */
    struct list_node *prev;  /* TYPE_POINTER to self */
};

/* Union within struct */
struct GTY(()) variant_container {
    int type;
    union {
        int int_value;
        double double_value;
        struct gcc_string *str_value;  /* TYPE_POINTER */
    } GTY((desc ("%0.type"))) data;
};

/* Array of unions */
union GTY(()) small_union_array[5];

/* Struct with array of pointers */
struct GTY(()) pointer_array_struct {
    void *pointers[20];  /* TYPE_ARRAY of TYPE_POINTER */
    int count;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) lang_struct {
    struct lang_struct *next;
    struct lang_struct *prev;
    int lang_specific_data;
};

/* Root structure containing pointers to all types */
struct GTY(()) root_container {
    /* TYPE_STRUCT references */
    struct base_struct *base;
    struct complex_struct *complex;
    
    /* TYPE_USER_STRUCT */
    struct user_struct *user;
    
    /* TYPE_UNION */
    union data_union *data_union_ptr;
    
    /* TYPE_ARRAY */
    int_array_t large_array;
    struct_ptr_array_t struct_ptrs;
    
    /* TYPE_STRING */
    struct gcc_string *string_data;
    
    /* TYPE_CALLBACK */
    callback_func_t callback;
    struct callback_container *callback_struct;
    
    /* Linked structures */
    struct list_node *list_head;
    struct list_node *list_tail;
    
    /* Variant container */
    struct variant_container *variant;
    
    /* Pointer array */
    struct pointer_array_struct *ptr_array;
    
    /* Language structure */
    struct lang_struct *lang_data;
    
    /* TYPE_UNDEFINED reference (non-GTY type) */
    struct undefined_struct *undefined;  /* Will trigger TYPE_UNDEFINED */
    
    /* Direct scalar members */
    scalar_int_t direct_int;
    scalar_double_t direct_double;
    enum color direct_color;
    
    /* Direct union */
    union data_union direct_union;
    
    /* Direct array */
    int direct_array[15];
    
    /* Direct callback */
    int (*direct_callback)(int, char*);
    
    /* For TYPE_POINTER coverage */
    int *int_ptr;
    char **string_ptr_ptr;
    void **void_ptr_array[8];
    
    /* Mixed array */
    union {
        int i;
        float f;
        char *s;
    } mixed_array[12];
};

/* Global root variable */
extern struct root_container GTY(()) global_root;

#endif /* GTY_TEST_H */
