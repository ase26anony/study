/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Forward declarations */
struct forward_declared_struct;

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct GTY(()) basic_struct {
    int id;
    char data;
};

/* TYPE_UNION: Basic union with GTY annotation */
union GTY(()) basic_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_POINTER: Struct containing pointers */
struct GTY(()) pointer_container {
    /* Regular pointer */
    struct basic_struct* GTY((skip)) regular_ptr;
    
    /* Pointer to forward declared struct */
    struct forward_declared_struct* GTY((skip)) forward_ptr;
    
    /* Pointer to self (recursive type) */
    struct pointer_container* GTY((skip)) self_ptr;
};

/* TYPE_ARRAY: Struct with arrays */
struct GTY(()) array_container {
    /* Fixed-size array */
    int GTY((length("10"))) fixed_array[10];
    
    /* Variable-length array (requires length option) */
    char* GTY((length("strlen($)"))) variable_array;
    
    /* Array of pointers */
    struct basic_struct* GTY((skip)) ptr_array[5];
};

/* TYPE_SCALAR: Various scalar types */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    double GTY((skip)) value;
    enum { RED, GREEN, BLUE } GTY((skip)) color;
};

/* TYPE_STRING: String types */
struct GTY(()) string_container {
    const char* GTY((skip)) name;
    char* GTY((skip)) buffer;
    const char* GTY((skip)) const_string;
};

/* TYPE_CALLBACK: Callback function types */
typedef void (*simple_callback)(int) GTY((callback));
typedef int (*complex_callback)(struct basic_struct*, void*) GTY((callback));

struct GTY(()) callback_container {
    simple_callback GTY((skip)) cb1;
    complex_callback GTY((skip)) cb2;
};

/* TYPE_USER_STRUCT: Forward declaration for user-defined type */
struct user_defined_type;

/* Template-like macro for generating multiple struct types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Instantiate template-like structs */
DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct basic_struct*);

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct basic_struct inner;
    union basic_union choice;
    struct pointer_container* GTY((skip)) ptr_field;
    struct array_container array_field;
};

/* Language-specific structure simulation */
struct GTY((tag("TS_VAR_DECL"))) lang_specific_struct {
    int decl_uid;
    const char* GTY((skip)) decl_name;
    struct lang_specific_struct* GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
