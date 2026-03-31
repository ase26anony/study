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

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) simple_struct {
    int field1;
    my_scalar field2;
};

struct GTY(()) nested_struct {
    struct simple_struct *inner;
    int count;
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
struct GTY((user)) user_defined_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: Union definitions */
union GTY(()) simple_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

union GTY(()) tagged_union {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct simple_struct *simple_struct_ptr;
typedef union simple_union * GTY(()) union_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct simple_struct struct_array[5];
typedef const char * GTY(()) string_array[3];

/* TYPE_CALLBACK: Function pointer typedef */
typedef int (*comparison_func)(const void *, const void *);
typedef void (* GTY(()) callback_func)(int, const char *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_feature;
    void *lang_data;
};
#endif

/* Additional complex structure to ensure thorough parsing */
struct GTY(()) complex_container {
    my_string_type name;
    simple_struct_ptr items;
    int_array counts;
    comparison_func compare;
    union simple_union data;
    struct undefined_type *future;  /* Pointer to undefined type */
};

#endif /* TEST_STRUCTURES_H */
