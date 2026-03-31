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
struct user_struct GTY((user)) {
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

/* TYPE_ARRAY: Array definitions */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_type)(int, void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific_struct GTY(()) {
    int lang_field1;
    void *lang_field2;
};
#endif

/* Nested structures for more coverage */
struct outer_struct GTY(()) {
    struct inner_struct GTY(()) {
        int inner_field;
        struct inner_struct *next;
    } inner;
    
    union nested_union GTY(()) {
        int a;
        double b;
    } uni;
    
    callback_type callback;
    int_array numbers;
};

/* Another user struct for additional coverage */
struct another_user_struct GTY((user)) {
    callback_type handler;
    my_string name;
};

/* Pointer to undefined type */
typedef struct undefined_type *undefined_ptr;

/* Array of pointers */
typedef struct my_struct *ptr_array[20];

/* Callback with complex signature */
typedef void (*complex_callback)(struct my_struct *, union my_union *, int_array);

#endif /* TEST_STRUCTURES_H */
