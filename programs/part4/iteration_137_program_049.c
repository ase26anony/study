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
struct GTY(()) regular_struct {
    int field1;
    double field2;
    struct regular_struct *next;
};

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct regular_struct *regular_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct regular_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_func)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    void *lang_data;
};
#endif

/* Nested structure to ensure traversal */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        struct inner_struct *next;
    } *inner;
    
    union GTY(()) nested_union {
        int a;
        double b;
    } u;
    
    int_array numbers;
    comparison_func compare;
};

/* Another structure with multiple pointer types */
struct GTY(()) complex_struct {
    my_string_type name;
    regular_struct_ptr rs_ptr;
    int_ptr numbers;
    struct undefined_type *undef_ptr;  /* TYPE_UNDEFINED pointer */
    void (*callback)(int, int);
};

#endif /* TEST_STRUCTURES_H */
