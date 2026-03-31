/* test_structures.h - Header with diverse type definitions for gengtype coverage */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic scalar typedefs */
typedef int my_scalar;
typedef unsigned int my_unsigned_scalar;
typedef char my_char_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct my_struct GTY(()) {
    int field1;
    char field2;
    struct my_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: Structure with GTY((user)) */
struct user_struct GTY((user)) {
    void *user_data;
    int user_id;
};

/* Another regular structure */
struct another_struct GTY(()) {
    double value;
    struct my_struct *link;
};

/* TYPE_UNION: Union definition */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char *string_val GTY((string));
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* Multi-dimensional array */
typedef double matrix[3][3];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific_struct GTY(()) {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structure for additional coverage */
struct outer_struct GTY(()) {
    struct inner_struct GTY(()) {
        int nested_field;
        union my_union nested_union;
    } inner;
    
    struct undefined_type *undef_ptr;  /* Pointer to undefined type */
    my_string name;                    /* String type */
    int_array numbers;                 /* Array type */
    comparison_fn compare;             /* Callback type */
};

/* Enumeration (should be treated as scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

/* Another union with GTY */
union data_union GTY(()) {
    my_scalar scalar_data;
    my_string string_data;
    struct my_struct *struct_data;
};

#endif /* TEST_STRUCTURES_H */
