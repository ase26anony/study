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
    struct my_struct* GTY((skip)) struct_ptr;
    
    /* Pointer to self */
    struct pointer_container* GTY((skip)) self_ptr;
    
    /* Void pointer */
    void* GTY((skip)) generic_ptr;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_array[10];
    
    /* Variable-length array (requires length callback) */
    int GTY((length("var_len"))) *variable_array;
    size_t var_len;
};

/* TYPE_SCALAR: Direct scalar type annotation */
typedef long GTY((skip)) my_long_type;

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) constant_string;
    char* GTY((skip)) mutable_string;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested structure for deep type graph */
struct nested_container GTY(()) {
    struct my_struct inner_struct;
    union my_union inner_union;
    struct pointer_container* GTY((skip)) ptr_member;
    struct array_container array_member;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for mutual recursion */
struct forward_decl;
struct recursive_struct GTY(()) {
    int value;
    struct forward_decl* GTY((skip)) next;
};

struct forward_decl GTY(()) {
    char tag;
    struct recursive_struct* GTY((skip)) link;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
