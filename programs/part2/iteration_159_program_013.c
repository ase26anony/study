/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct my_struct;
union my_union;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int id;
    char *name;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void *ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
    struct my_struct * GTY((skip)) struct_ptr;
    union my_union * GTY((skip)) union_ptr;
    void * GTY((skip)) generic_ptr;
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) fixed_array[10];
    struct my_struct * GTY((length("5"))) struct_array[5];
};

/* TYPE_SCALAR: Struct with scalar types */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    short GTY((skip)) index;
};

/* TYPE_STRING: Struct with string fields */
struct string_container GTY(()) {
    const char * GTY((skip)) name;
    char * GTY((skip)) description;
    const char * GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
    void * GTY((skip)) user_data;
};

/* Complex nested structure for type graph testing */
struct complex_node GTY(()) {
    int value;
    struct complex_node * GTY((skip)) next;
    struct complex_node * GTY((skip)) prev;
    union my_union GTY((tag("0"))) data;
};

/* Template-like macro for generating multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Language-specific structure hook (simulating Tree nodes) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
    int decl_uid;
    const char * GTY((skip)) decl_name;
    struct lang_struct * GTY((skip)) chain;
};

/* User-defined struct type forward declaration */
struct user_defined_type;

#endif /* TEST_GTY_H */
