/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct my_struct;
union my_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int id;
    const char* GTY((skip)) name;  /* TYPE_STRING */
    struct my_struct* GTY((skip)) next;  /* TYPE_POINTER */
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    struct my_struct* GTY((skip)) struct_ptr;
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) arr[10];  /* TYPE_ARRAY */
    int count;
};

/* TYPE_SCALAR: Explicit scalar type with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;  /* TYPE_SCALAR */
    unsigned GTY((skip)) flags;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn handler;
    void* GTY((skip)) user_data;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Instantiate template-like structs */
DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Complex nested type graph */
struct graph_node GTY(()) {
    int id;
    struct graph_node* GTY((skip)) children[5];  /* Array of pointers */
    struct graph_node* GTY((skip)) parent;
};

/* Union containing struct and pointer */
union complex_union GTY(()) {
    struct my_struct nested_struct;
    struct graph_node* node_ptr;
    callback_fn callback;
};

/* Language-specific structure simulation */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {  /* TYPE_LANG_STRUCT hint */
    int decl_uid;
    const char* GTY((skip)) identifier;
    struct lang_specific* GTY((skip)) chain;
};

/* User-defined struct type reference */
struct user_defined_container GTY(()) {
    struct user_struct_type* GTY((skip)) user_data;  /* TYPE_USER_STRUCT */
    int regular_field;
};

#endif /* TEST_GTY_H */
