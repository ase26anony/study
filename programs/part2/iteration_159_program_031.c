#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) base_struct {
    int id;
    char data;
};

/* TYPE_UNION: Basic union with GTY annotation */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_container {
    /* Pointer to another GTY-annotated struct */
    struct base_struct* GTY((skip)) child;
    
    /* Pointer to self (recursive type) */
    struct pointer_container* GTY((skip)) next;
    
    /* Void pointer */
    void* GTY((skip)) opaque_data;
};

/* TYPE_ARRAY: Struct with arrays */
struct GTY(()) array_container {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_arr[10];
    
    /* Array of pointers */
    struct base_struct* GTY((skip)) ptr_arr[5];
    
    /* Multi-dimensional array */
    char GTY((length("20"))) matrix[4][5];
};

/* TYPE_SCALAR: Various scalar types with GTY */
struct GTY(()) scalar_types {
    int GTY((skip)) counter;
    long GTY((skip)) big_counter;
    unsigned GTY((skip)) flags;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    const char* GTY((skip)) name;
    char* GTY((skip)) mutable_str;
    const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct GTY(()) callback_container {
    callback_func GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested structure for deep type graph */
struct GTY(()) complex_node {
    struct complex_node* GTY((skip)) left;
    struct complex_node* GTY((skip)) right;
    union data_union GTY((tag("0"))) data;
    struct array_container arrays;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { \
    T first; \
    T second; \
} GTY(())

/* Instantiate template-like macros */
DEF_PAIR(int);
DEF_PAIR(float);
DEF_PAIR(struct base_struct*);

/* Forward declaration for mutual recursion */
struct forward_decl;
struct GTY(()) recursive_container {
    struct forward_decl* GTY((skip)) fwd_ptr;
    struct recursive_container* GTY((skip)) self_ptr;
};

struct GTY(()) forward_decl {
    int value;
    struct recursive_container* GTY((skip)) back_ref;
};

#endif /* TEST_GTY_H */
