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
    struct undefined_type *forward_ref;  /* Uses undefined type */
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
typedef union my_union * GTY(()) my_union_ptr;
typedef int *int_ptr;

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct my_struct GTY(()) struct_array[5];
typedef const char * GTY((string)) string_array[3];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, const char *);
typedef void (* GTY(()) gty_callback)(struct my_struct *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct GTY(()) lang_specific_struct {
    int lang_field;
    void *lang_data;
};
#endif

/* Nested and complex types for thorough testing */
struct GTY(()) complex_container {
    my_struct_ptr ptr_field;
    int_array array_field;
    callback_type callback_field;
    union {
        int option1;
        float option2;
    } GTY(()) anonymous_union;
    struct {
        int nested1;
        float nested2;
    } GTY(()) anonymous_struct;
};

/* Another user struct with different configuration */
struct GTY((user)) another_user_struct {
    long id;
    char * GTY((string)) name;
};

/* Pointer to undefined type */
typedef struct undefined_type *undefined_ptr;

/* Array of pointers */
typedef struct my_struct * GTY(()) ptr_array[8];

/* Callback returning pointer */
typedef struct my_struct * (*struct_factory)(int);

#endif /* TEST_STRUCTURES_H */
