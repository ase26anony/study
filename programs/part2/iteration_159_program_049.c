/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct my_struct;
union my_union;

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
    /* Pointer to another GTY-annotated struct */
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

/* TYPE_SCALAR: Direct scalar types with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;
    char* GTY((skip)) mutable_string;
    const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Complex nested structure for type graph testing */
struct complex_node GTY(()) {
    int value;
    struct complex_node* GTY((skip)) next;
    struct complex_node* GTY((skip)) prev;
    union my_union GTY((skip)) data;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Language-specific structure simulation */
#ifdef __cplusplus
/* TYPE_LANG_STRUCT: Simulating Tree node for GCC frontend */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
    int code;
    union {
        int int_val;
        double real_val;
        const char* string_val;
    } GTY((desc("%1.code"))) u;
};
#endif

#endif /* TEST_GTY_H */
