/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct my_struct;
union my_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int id;
    const char * GTY((skip)) name;  /* TYPE_STRING */
    struct my_struct * GTY((skip)) next;  /* TYPE_POINTER */
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    struct my_struct * GTY((skip)) struct_ptr;  /* TYPE_POINTER inside union */
};

/* TYPE_ARRAY: Struct with array field */
struct array_container GTY(()) {
    int GTY((length("10"))) arr[10];  /* TYPE_ARRAY */
    int count;
};

/* TYPE_SCALAR: Struct with scalar field */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;  /* TYPE_SCALAR */
    unsigned flags;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { \
    T first; \
    T second; \
} GTY(())

/* Instantiate template-like structs */
DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

/* Struct using callback type */
struct callback_container GTY(()) {
    callback_fn handler;
    int data;
};

/* Complex nested structure */
struct nested_container GTY(()) {
    struct my_struct base;
    union my_union variant;
    struct array_container arrays;
    struct scalar_container scalars;
    struct pair_int int_pair;
    struct callback_container callbacks;
};

/* Forward declaration for mutual recursion */
struct tree_node;

/* Tree structure with mutual recursion */
struct tree_node GTY(()) {
    int value;
    struct tree_node * GTY((skip)) left;   /* TYPE_POINTER */
    struct tree_node * GTY((skip)) right;  /* TYPE_POINTER */
    struct tree_node * GTY((skip)) parent; /* TYPE_POINTER */
};

/* Language-specific structure (simulating Tree nodes) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
    int code;
    const char * GTY((skip)) identifier;
    struct lang_struct * GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
