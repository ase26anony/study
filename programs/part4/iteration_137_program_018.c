/* test_structures.h - Contains examples of all type categories tracked by gengtype */

#ifndef TEST_STRUCTURES_H
#define TEST_STRUCTURES_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_type;

/* TYPE_SCALAR: Basic typedefs */
typedef int my_scalar;
typedef unsigned long my_other_scalar;

/* TYPE_STRING: String type with GTY((string)) */
typedef const char * GTY((string)) my_string_type;

/* TYPE_STRUCT: Regular structures with GTY(()) */
struct GTY(()) my_struct {
    int field1;
    my_scalar field2;
    struct undefined_type *forward_ref;  /* Reference to undefined type */
};

/* TYPE_USER_STRUCT: Structure marked with GTY((user)) */
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

/* TYPE_POINTER: Typedefs for pointers */
typedef struct my_struct *my_struct_ptr;
typedef union my_union *my_union_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array type definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_specific_field;
    comparison_fn callback_field;  /* Includes callback type */
};
#endif

/* Nested structures to ensure thorough traversal */
struct GTY(()) outer_struct {
    struct GTY(()) inner_struct {
        int inner_field;
        my_string_type name;  /* String type field */
    } nested;
    
    union GTY(()) inner_union {
        int option1;
        float option2;
    } choice;
    
    int_array numbers;  /* Array type field */
    comparison_fn sorter;  /* Callback field */
};

/* Another structure with pointer chain */
struct GTY(()) pointer_chain {
    struct pointer_chain * GTY((skip)) next;  /* Pointer with skip option */
    my_struct_ptr data;
};

#endif /* TEST_STRUCTURES_H */
