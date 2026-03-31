/* test_structures.h - Header with diverse type definitions for gengtype coverage */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String type with GTY annotation */
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structures with GTY annotations */
struct my_struct GTY(()) {
    int field1;
    double field2;
    struct my_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct user_defined GTY((user)) {
    void *data;
    int size;
};

/* TYPE_UNION: Union definition */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char *string_val GTY((string));
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *struct_ptr;
typedef int *int_ptr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_func)(int, void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific GTY(()) {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structures for comprehensive coverage */
struct outer_struct GTY(()) {
    struct inner_struct GTY(()) {
        int inner_field;
        struct inner_struct *next;
    } inner;
    
    union nested_union GTY(()) {
        int a;
        double b;
    } u;
    
    int_array array_field;
    callback_func callback_field;
};

/* More complex type combinations */
typedef struct complex_type GTY(()) {
    my_scalar scalar_field;
    my_string string_field;
    struct_ptr struct_ptr_field;
    int_array array_field;
    callback_func callback_field;
    
    union {
        int option1;
        double option2;
    } variant;
} complex_type_t;

/* Template-like macro for generating types */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct name##_container GTY(()) { \
        field_type value; \
        struct name##_container *next; \
    }

DECLARE_GTY_STRUCT(int, int);
DECLARE_GTY_STRUCT(double, double);

/* Enumeration type (should be treated as scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum;

/* Function pointer with parameters */
typedef void (*event_handler)(int event_id, void *user_data);

/* Self-referential structure */
struct tree_node GTY(()) {
    int data;
    struct tree_node *left;
    struct tree_node *right;
    struct tree_node *parent;
};

/* Structure with array of pointers */
struct pointer_array_container GTY(()) {
    void *pointers[8];
    int count;
};

#endif /* TEST_STRUCTURES_H */
