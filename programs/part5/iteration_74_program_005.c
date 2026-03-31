#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Incomplete/opaque struct declaration */
struct opaque;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef double scalar_double;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_t)(int);
typedef void (*complex_callback_t)(struct opaque *, int);

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
    int id;
    scalar_int value;
    scalar_double dvalue;
};

/* TYPE_ARRAY: Struct containing arrays */
struct GTY(()) array_struct {
    int numbers[10];
    struct base_struct * GTY((skip)) ptr_array[5];
    callback_t callbacks[3];
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    struct base_struct * GTY((tag("0"))) base_ptr;
    struct array_struct * GTY((tag("1"))) array_ptr;
    long long int_data;
    double float_data;
    callback_t func_ptr;
};

/* TYPE_POINTER: Struct focusing on pointer members */
struct GTY(()) pointer_struct {
    struct base_struct * GTY((skip)) direct_ptr;
    struct opaque * GTY((skip)) opaque_ptr;
    union data_union * GTY((skip)) union_ptr;
    struct pointer_struct * GTY((skip)) next;  /* Self-referential pointer */
    void * GTY((skip)) generic_ptr;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_struct {
    int user_id;
    char * GTY((skip)) user_name;
    struct base_struct * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific structure (mimicking GCC internal) */
struct GTY(()) lang_tree_node {
    int code;
    union data_union GTY((desc("%1.code"))) data;
    struct lang_tree_node * GTY((skip)) left;
    struct lang_tree_node * GTY((skip)) right;
    struct lang_tree_node * GTY((skip)) parent;
};

/* TYPE_STRING: Struct with string pointer */
struct GTY(()) string_struct {
    const char * GTY((skip)) name;
    char * GTY((skip)) buffer;
    int length;
};

/* Complete the opaque struct definition */
struct GTY(()) opaque {
    int secret;
    struct base_struct * GTY((skip)) internal;
    callback_t notify;
};

/* Top-level complex struct containing all type variations */
struct GTY(()) top_level {
    /* Scalar members */
    int version;
    scalar_int count;
    scalar_double total;
    
    /* Struct pointers */
    struct base_struct * GTY((skip)) base;
    struct array_struct * GTY((skip)) arrays;
    struct pointer_struct * GTY((skip)) pointers;
    struct string_struct * GTY((skip)) strings;
    
    /* Union */
    union data_union data;
    
    /* Arrays */
    struct base_struct * GTY((skip)) struct_array[8];
    int int_array[20];
    callback_t callback_array[4];
    
    /* Language structure */
    struct lang_tree_node * GTY((skip)) lang_node;
    
    /* User struct */
    struct user_struct * GTY((skip)) user;
    
    /* Opaque pointer */
    struct opaque * GTY((skip)) hidden;
    
    /* Callback function pointer */
    callback_t handler;
    complex_callback_t complex_handler;
    
    /* Self-reference for pointer chains */
    struct top_level * GTY((skip)) next;
    struct top_level * GTY((skip)) prev;
    
    /* Union array */
    union data_union union_array[3];
    
    /* Mixed array */
    void * GTY((skip)) mixed_array[5];
};

/* Root variable for gengtype to start traversal */
extern GTY(()) struct top_level *global_root;

/* Additional root variables to ensure coverage */
extern GTY(()) struct lang_tree_node *lang_root;
extern GTY(()) union data_union global_union;
extern GTY(()) struct user_struct *user_root;

#endif /* TEST_GTY_H */
