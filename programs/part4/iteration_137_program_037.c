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

/* TYPE_USER_STRUCT: User-defined structure with GTY((user)) */
struct user_defined GTY((user)) {
    void *data;
    int tag;
};

/* TYPE_UNION: Union types */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct *my_struct_ptr;
typedef int *int_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_ARRAY: Array types */
typedef int int_array[10];
typedef struct my_struct struct_array[5];

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_LANG_STRUCT: Language-specific structure */
#ifdef GCC
struct lang_specific GTY(()) {
    int lang_field;
    void *lang_data;
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
        float b;
    } nested;
    
    my_string name;
    comparison_fn compare;
    int_array numbers;
};

/* Another union with GTY */
union tagged_union GTY(()) {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
};

#endif /* TEST_STRUCTURES_H */
