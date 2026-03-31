/* Test header for gengtype-state.cc coverage */
#ifndef GTY_TEST_H
#define GTY_TEST_H

#include "gtype-desc.h"

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef double scalar_double_t;

/* TYPE_ENUM (treated as scalar) */
enum color { RED, GREEN, BLUE };

/* TYPE_STRING: String-like structure */
struct GTY(()) gcc_string {
    int length;
    char * GTY((skip)) data;
};

/* TYPE_STRUCT: Basic annotated struct */
struct GTY(()) base_struct {
    int id;
    char name[32];  /* TYPE_ARRAY: Fixed-size array */
    struct base_struct *next;  /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    void * GTY((skip)) user_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int int_val;
    double double_val;
    char *string_val;
    struct base_struct *struct_ptr;
};

/* TYPE_ARRAY: Typedef for array type */
typedef int int_array_t[10];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func_t)(void *context, int value);

/* TYPE_POINTER: Various pointer types */
typedef struct base_struct *base_ptr_t;
typedef int *int_ptr_t;
typedef callback_func_t callback_ptr_t;

/* Complex nested structure to trigger deep traversal */
struct GTY(()) complex_node {
    int id;
    
    /* TYPE_ARRAY of pointers */
    struct base_struct * GTY((length("5"))) ptr_array[5];
    
    /* TYPE_UNION */
    union data_union data;
    
    /* TYPE_CALLBACK as member */
    callback_func_t callback;
    
    /* TYPE_STRING */
    struct gcc_string str;
    
    /* TYPE_POINTER to user struct */
    struct user_struct *user;
    
    /* TYPE_ARRAY of scalars */
    int scores[8];
    
    /* Linked list for chaining */
    struct complex_node *next;
    struct complex_node *prev;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((desc("%0.lang_code"))) lang_struct {
    int lang_code;
    void * GTY((skip)) lang_data;
    struct lang_struct *next_lang;
};

/* Root structure containing all types */
struct GTY(()) root_container {
    /* TYPE_STRUCT references */
    struct base_struct base;
    struct complex_node complex;
    
    /* TYPE_UNION */
    union data_union main_data;
    
    /* TYPE_POINTER to various types */
    struct user_struct *user_ptr;
    struct lang_struct *lang_ptr;
    struct gcc_string *string_ptr;
    
    /* TYPE_ARRAY of structs */
    struct base_struct struct_array[3];
    
    /* TYPE_ARRAY of pointers */
    struct complex_node *node_ptrs[4];
    
    /* TYPE_CALLBACK */
    callback_func_t handlers[2];
    
    /* TYPE_SCALAR members */
    scalar_int_t counter;
    enum color current_color;
    
    /* TYPE_STRING */
    struct gcc_string title;
    
    /* Chain for traversal */
    struct root_container *next_root;
};

/* Non-annotated types (may become TYPE_UNDEFINED) */
struct plain_struct {
    int x, y;
    char label[20];
};

typedef struct plain_struct *plain_ptr_t;

/* Another callback type */
typedef void (*simple_callback_t)(int);

/* Mixed pointer types in a struct */
struct GTY(()) pointer_collector {
    int *int_ptr;
    char **string_ptr_ptr;
    struct plain_struct *plain_ptr;  /* Non-GTY struct pointer */
    simple_callback_t simple_cb;
};

/* Array of unions */
union GTY(()) small_union {
    int i;
    float f;
    char c;
};

struct GTY(()) union_container {
    union small_union items[16];
    int count;
};

/* Self-referential structure for deep graphs */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Container with all types */
struct GTY(()) type_manifest {
    /* Basic types */
    int scalar;
    double precise;
    
    /* String */
    struct gcc_string name;
    
    /* Struct */
    struct base_struct base;
    
    /* Union */
    union data_union variant;
    
    /* Array */
    int matrix[4][4];
    
    /* Pointer */
    void *generic_ptr;
    
    /* Callback */
    callback_func_t validator;
    
    /* Language struct */
    struct lang_struct *lang_info;
    
    /* User struct */
    struct user_struct *user_info;
    
    /* Chain */
    struct type_manifest *next;
};

#endif /* GTY_TEST_H */
