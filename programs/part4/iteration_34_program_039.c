#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar GTY(());
typedef unsigned int my_unsigned_scalar;

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Plain C structs */
struct my_struct GTY(()) {
    int field1;
    char field2;
    struct my_struct *next GTY((skip));
};

/* TYPE_USER_STRUCT: Struct with user-defined marker */
struct user_struct GTY((user)) {
    void *data;
    int tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    char *str_val GTY((length("str_len")));
    double dbl_val;
    int str_len;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(()) {
    struct my_struct *struct_ptr;
    union my_union *union_ptr;
    int *scalar_ptr;
    char **double_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(()) {
    int fixed_array[10];
    char *variable_array GTY((length("array_len")));
    int array_len;
    struct my_struct *struct_array GTY((length("struct_count")));
    int struct_count;
};

/* TYPE_STRING: String type with length annotation */
struct string_container GTY(()) {
    char *string_field GTY((length("str_length")));
    int str_length;
    const char *const_string GTY((length("const_len")));
    int const_len;
};

/* TYPE_CALLBACK: Callback/function pointer types */
typedef void (*callback_func)(int, char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void (*direct_func)(void) GTY((callback));
    int (*comparator)(const void *, const void *);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct)) {
    int lang_field1;
    void *lang_field2 GTY((skip));
};

/* Complex nested type to ensure thorough traversal */
struct complex_nested GTY(()) {
    struct pointer_container *ptr_container;
    struct array_container arrays;
    struct string_container strings;
    union my_union data_union;
    callback_func callback;
    struct lang_specific_struct *lang_struct_ptr;
    
    /* Self-referential pointer */
    struct complex_nested *next GTY((skip));
    
    /* Array of pointers */
    struct my_struct *struct_ptrs[5];
    
    /* Variable length array at end */
    int flexible_array[] GTY((length("flex_len")));
    int flex_len;
};

/* Another union with GTY tags on members */
union tagged_union GTY(()) {
    struct my_struct *s GTY((tag("0")));
    struct user_struct *u GTY((tag("1")));
    struct array_container *a GTY((tag("2")));
    int tag_value;
};

/* Enumeration type (also scalar) */
typedef enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
} my_enum GTY(());

/* Struct with bitfields */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Opaque pointer type */
typedef struct my_struct *opaque_handle GTY(());

/* Template-like macro for generating typed containers */
#define DECLARE_CONTAINER(TYPE, NAME) \
    struct NAME ## _container GTY(()) { \
        TYPE *items GTY((length("count"))); \
        int count; \
        int capacity; \
    }

/* Instantiate some template containers */
DECLARE_CONTAINER(int, int);
DECLARE_CONTAINER(struct my_struct, my_struct);
DECLARE_CONTAINER(char, char);

/* Forward declaration that will remain undefined (TYPE_UNDEFINED) */
struct never_defined GTY(());

#endif /* TEST_TYPES_H */
