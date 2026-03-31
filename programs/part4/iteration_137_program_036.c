/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_unsigned_scalar;
typedef double my_float_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY annotation */
struct GTY(()) simple_struct {
    int field1;
    double field2;
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_defined_struct {
    void *data;
    int size;
};

/* Nested structure for more complexity */
struct GTY(()) outer_struct {
    struct simple_struct inner;
    struct user_defined_struct *user_ptr;
};

/* TYPE_UNION: Union definitions */
union GTY(()) my_union {
    int as_int;
    double as_double;
    void *as_pointer;
};

/* Another union without GTY for variety */
union plain_union {
    float f;
    char c[4];
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct simple_struct *simple_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct simple_struct struct_array[5];
typedef const char *string_array[3];

/* Multi-dimensional array */
typedef double matrix[3][3];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_func)(const void *, const void *);

/* More complex callback with GTY */
typedef void (*GTY((callback)) event_handler)(int event_id, void *data);

/* Language-specific structure (TYPE_LANG_STRUCT) */
#ifdef GCC
/* This simulates a language-specific structure definition */
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    void *lang_data;
};
#endif

/* Additional structures to ensure coverage */

/* Structure containing pointers */
struct GTY(()) pointer_container {
    int *int_ptr;
    char **string_ptr_ptr;
    struct simple_struct *struct_ptr;
};

/* Structure with callback field */
struct GTY(()) callback_container {
    comparison_func compare;
    event_handler handler;
};

/* Structure with array field */
struct GTY(()) array_container {
    int_array numbers;
    string_array names;
};

/* Union with GTY inside structure */
struct GTY(()) union_container {
    int type;
    union {
        int int_value;
        double double_value;
        char *string_value;
    } data;
};

/* Forward declared structure that gets defined later */
struct forward_declared;

/* Now define it */
struct GTY(()) forward_declared {
    int id;
    struct forward_declared *next;
};

/* Enumeration type (should be treated as scalar) */
typedef enum {
    RED,
    GREEN,
    BLUE
} color_enum;

/* Const pointer typedef */
typedef const int *const_int_ptr;

/* Volatile pointer */
typedef volatile int *volatile_int_ptr;

#endif /* TEST_STRUCTURES_H */
