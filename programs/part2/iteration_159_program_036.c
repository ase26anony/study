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
    /* Regular pointer */
    struct my_struct* GTY((skip)) struct_ptr;
    
    /* Pointer to union */
    union my_union* GTY((skip)) union_ptr;
    
    /* Self-referential pointer */
    struct pointer_container* GTY((skip)) next;
};

/* TYPE_ARRAY: Struct with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_arr[10];
    
    /* Array of pointers */
    struct my_struct* GTY((length("5"))) ptr_arr[5];
    
    /* Two-dimensional array */
    double GTY((length("3*4"))) matrix[3][4];
};

/* TYPE_SCALAR: Various scalar types */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    short GTY((skip)) index;
    char GTY((skip)) code;
};

/* TYPE_STRING: String types */
struct string_container GTY(()) {
    const char* GTY((skip)) name;
    char* GTY((skip)) buffer;
    const char* GTY((skip)) path;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
    void* GTY((skip)) user_data;
};

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Generate multiple type instances */
DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Complex nested type */
struct nested_container GTY(()) {
    struct my_struct base;
    union my_union variant;
    struct pointer_container* GTY((skip)) ptr_field;
    struct array_container array_field;
};

/* Language-specific structure simulation */
#ifdef __cplusplus
/* TYPE_LANG_STRUCT: Simulating tree nodes with tags */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
    int lang_specific_field;
    void* GTY((skip)) tree_node;
};

struct another_lang_struct GTY((tag("TS_FUNCTION_DECL"))) {
    const char* GTY((skip)) name;
    int line_number;
};
#endif

/* User-defined type marker (requires gtype-desc.cc) */
#ifdef USER_STRUCT_TYPE_DEFINED
struct user_defined_type;
#endif

#endif /* TEST_GTY_H */
