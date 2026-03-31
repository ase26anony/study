/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
    /* Pointer to another GTY struct */
    struct my_struct* GTY((skip)) child;
    
    /* Pointer to self */
    struct pointer_container* GTY((skip)) next;
    
    /* Void pointer */
    void* GTY((skip)) data;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_arr[10];
    
    /* Variable-length array (requires length expression) */
    char* GTY((length("strlen(name)"))) dynamic_arr;
    
    /* Array of pointers */
    struct my_struct* GTY((skip)) GTY((length("ptr_count"))) ptr_arr[5];
    int ptr_count;
};

/* TYPE_SCALAR: Direct scalar with GTY (less common but valid) */
extern long GTY((skip)) global_counter;

/* TYPE_STRING: String fields */
struct string_container GTY(()) {
    const char* GTY((skip)) name;
    char* GTY((skip)) description;
    
    /* Array of strings */
    const char** GTY((skip)) GTY((length("num_tags"))) tags;
    int num_tags;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested type for TYPE_STRUCT recursion */
struct tree_node GTY(()) {
    int value;
    struct tree_node* GTY((skip)) left;
    struct tree_node* GTY((skip)) right;
    struct tree_node* GTY((skip)) parent;
};

/* Union containing struct and pointer */
union complex_union GTY(()) {
    struct my_struct s;
    struct pointer_container* GTY((skip)) pc;
    long scalar;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T, NAME) \
    struct NAME { \
        T first; \
        T second; \
    } GTY(())

/* Instantiate template-like types */
DEF_PAIR(int, int_pair);
DEF_PAIR(double, double_pair);
DEF_PAIR(struct my_struct*, struct_ptr_pair);

/* Forward declaration for mutual recursion */
struct forward_decl_struct;

struct recursive_container GTY(()) {
    struct forward_decl_struct* GTY((skip)) fwd_ptr;
    int id;
};

struct forward_decl_struct GTY(()) {
    struct recursive_container* GTY((skip)) container;
    char data[32];
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
