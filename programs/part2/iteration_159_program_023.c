#ifndef TEST_GTY_H
#define TEST_GTY_H

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
    /* Regular pointer */
    struct my_struct* GTY((skip)) regular_ptr;
    
    /* Pointer to self */
    struct pointer_container* GTY((skip)) self_ptr;
    
    /* Pointer to union */
    union my_union* GTY((skip)) union_ptr;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_arr[10];
    
    /* Array of pointers */
    struct my_struct* GTY((skip)) ptr_arr[5];
    
    /* Multi-dimensional array */
    double GTY((length("3*4"))) matrix[3][4];
};

/* TYPE_SCALAR: Direct scalar type annotation */
typedef long GTY((skip)) my_long_type;

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;
    char* GTY((skip)) mutable_str;
    const char* GTY((skip)) const_string_array[3];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_fn handler;
    void* GTY((skip)) user_data;
};

/* Complex nested type for recursive processing */
struct complex_node GTY(()) {
    int value;
    struct complex_node* GTY((skip)) left;
    struct complex_node* GTY((skip)) right;
    union my_union data;
    struct array_container arrays;
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for mutual recursion */
struct forward_declared GTY(());

struct recursive_container GTY(()) {
    struct forward_declared* GTY((skip)) fwd_ptr;
    struct recursive_container* GTY((skip)) next;
};

struct forward_declared GTY(()) {
    int id;
    struct recursive_container* GTY((skip)) container;
};

/* Language-specific structure simulation */
struct lang_specific_node GTY((tag("TS_VAR_DECL"))) {
    int decl_type;
    const char* GTY((skip)) name;
    struct lang_specific_node* GTY((skip)) chain;
};

/* User-defined struct type marker */
struct user_defined_type;

#endif /* TEST_GTY_H */
