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
typedef const char *my_string GTY((string));

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct my_struct GTY(()) {
    int field1;
    double field2;
    struct my_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct user_defined_struct GTY((user)) {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: Union definition */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void *as_pointer;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef struct my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific_struct GTY(()) {
    int lang_field1;
    void *lang_field2 GTY((skip));
};
#endif

/* Additional complex types to ensure thorough parsing */

/* Nested structure */
struct outer_struct GTY(()) {
    struct inner_struct GTY(()) {
        int inner_field;
        struct inner_struct *next GTY((skip));
    } inner;
    int outer_field;
};

/* Union within structure */
struct with_union GTY(()) {
    int type;
    union {
        int int_value;
        double double_value;
        char *string_value GTY((string));
    } data;
};

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Callback with arguments */
typedef void (*event_handler)(int event_id, void *data);

/* Another user structure for good measure */
struct another_user_struct GTY((user)) {
    char *name GTY((string));
    int priority;
};

#endif /* TEST_STRUCTURES_H */
