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

/* TYPE_STRUCT: Regular struct with GTY(()) */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ref;  /* Uses undefined type */
};

/* TYPE_USER_STRUCT: Struct with GTY((user)) */
struct GTY((user)) user_struct {
    void *opaque_data;
    int user_tag;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int as_int;
    float as_float;
    void *as_pointer;
};

/* TYPE_POINTER: Typedef for pointer types */
typedef struct my_struct *my_struct_ptr;
typedef union my_union * GTY(()) my_union_ptr;
typedef int *int_ptr;

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct GTY(()) struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(int, const char *);
typedef int (* GTY(()) another_callback)(void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    void *lang_data;
};
#endif

/* Nested structures for additional coverage */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        my_string_type name;
    } inner;
    
    union GTY(()) inner_union {
        int option_a;
        float option_b;
    } choice;
    
    callback_func handler;
    int_array numbers;
};

/* More complex examples */
struct GTY(()) complex_example {
    /* Pointer to scalar */
    int *scalar_ptr;
    
    /* Pointer to struct */
    struct outer_struct *outer_ptr;
    
    /* Array of pointers */
    callback_func handlers[8];
    
    /* Nested array */
    int matrix[3][3];
    
    /* String array */
    const char * GTY((string)) strings[4];
    
    /* Self-referential pointer */
    struct complex_example *next;
};

/* Enumeration (should be treated as scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

/* Function pointer with complex signature */
typedef struct my_struct *(*factory_func)(int, const char * GTY((string)));

#endif /* TEST_STRUCTURES_H */
