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
    my_scalar field2;
    struct undefined_type *ptr_field;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct GTY((user)) user_struct {
    void *user_data;
    int user_id;
};

/* Another regular struct for variety */
struct GTY(()) another_struct {
    my_string_type name;
    int value;
};

/* TYPE_UNION: Union definition */
union GTY(()) my_union {
    int int_val;
    double double_val;
    my_string_type string_val;
};

/* TYPE_POINTER: Typedefs for pointers */
typedef regular_struct *regular_struct_ptr;
typedef my_union *union_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef regular_struct struct_array[5];
typedef const char * GTY((string)) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_callback)(const void *, const void *);

/* Nested structure for complexity */
struct GTY(()) outer_struct {
    regular_struct inner;
    union_ptr optional_union;
    int_array numbers;
    comparison_callback compare_func;
};

/* Language-specific structure - TYPE_LANG_STRUCT */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    void *lang_data;
};
#endif

/* Additional pointer types with GTY */
typedef struct GTY(()) regular_struct * GTY(()) gty_struct_ptr;
typedef union GTY(()) my_union * GTY(()) gty_union_ptr;

/* Array of pointers */
typedef regular_struct *struct_ptr_array[8];

/* Callback in a structure */
struct GTY(()) callback_container {
    comparison_callback cb;
    void *user_data;
};

/* Complex nested example */
struct GTY(()) complex_example {
    struct GTY(()) {
        int x;
        int y;
    } point;
    
    union GTY(()) {
        int i;
        double d;
    } value;
    
    string_array strings;
    struct_ptr_array pointers;
};

#endif /* TEST_STRUCTURES_H */
