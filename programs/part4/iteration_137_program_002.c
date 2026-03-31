/* test_structures.h - Contains examples of all gengtype type categories */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ref;  /* Uses undefined type */
};

/* TYPE_USER_STRUCT: Struct with GTY((user)) */
struct GTY((user)) user_struct {
    void *data;
    int (*process)(void *);
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int as_int;
    float as_float;
    char * GTY((string)) as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef my_struct *struct_ptr;
typedef int *int_ptr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct GTY(()) my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    void *lang_data;
};
#endif

/* Another struct to ensure multiple instances are counted */
struct GTY(()) another_struct {
    callback_type callback;
    int_array numbers;
};

/* Nested structure for complex type graph */
struct GTY(()) container {
    struct my_struct *first;
    union my_union value;
    callback_type handler;
    struct GTY((skip)) {  /* Non-GTY nested struct */
        int internal;
    } internal_data;
};

#endif /* TEST_STRUCTURES_H */
