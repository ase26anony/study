/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedef */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with GTY((user)) */
struct GTY((user)) my_user_struct {
    void *data;
    int size;
};

/* Another regular struct */
struct GTY(()) another_struct {
    my_struct *nested;
    int count;
};

/* TYPE_UNION: Union with GTY annotation */
union GTY(()) my_union {
    int int_val;
    float float_val;
    my_scalar scalar_val;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct * GTY(()) my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (* GTY((callback)) my_callback)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structure to test deeper type traversal */
struct GTY(()) container_struct {
    my_struct members[4];          /* TYPE_ARRAY of TYPE_STRUCT */
    my_union optional;             /* TYPE_UNION */
    my_callback handler;           /* TYPE_CALLBACK */
    my_string_type name;           /* TYPE_STRING */
    struct undefined_type *future; /* TYPE_POINTER to TYPE_UNDEFINED */
};

/* Enumeration (should be treated as scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

/* Complex nested example */
struct GTY(()) complex_example {
    /* Array of pointers to structs */
    my_struct * GTY((length("count"))) items;
    
    /* Callback function */
    my_callback GTY((skip)) notify;
    
    /* Union with tag */
    union GTY(()) {
        int as_int;
        float as_float;
    } value;
    
    /* String field */
    my_string_type description;
    
    /* Scalar fields */
    my_scalar id;
    my_enum status;
    
    /* Pointer to user struct */
    struct my_user_struct * GTY((user)) user_data;
};

#endif /* TEST_STRUCTURES_H */
