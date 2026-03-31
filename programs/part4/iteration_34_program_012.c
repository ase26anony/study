#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY((user));

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY((tag("UNDEF")));

/* TYPE_STRING: String type with length callback */
struct string_struct {
    char *data GTY((length("strlen($1->data) + 1")));
    int length;
} GTY((tag("STRING_STRUCT")));

/* TYPE_STRUCT: Regular struct */
struct regular_struct {
    int field1;
    float field2;
    struct regular_struct *next GTY((skip));
} GTY((tag("REGULAR_STRUCT")));

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_defined {
    int id;
    void *data GTY((user));
} GTY((user));

/* TYPE_UNION: Union type */
union my_union {
    int int_val;
    float float_val;
    char *str_val GTY((length("strlen($1->str_val) + 1")));
} GTY((tag("MY_UNION")));

/* TYPE_POINTER: Pointer types */
struct pointer_struct {
    struct regular_struct *regular_ptr;
    struct user_defined *user_ptr GTY((skip));
    union my_union *union_ptr;
    int *scalar_ptr;
} GTY((tag("POINTER_STRUCT")));

/* TYPE_ARRAY: Array types */
struct array_struct {
    int fixed_array[10];
    int *variable_array GTY((length("$1->array_length")));
    size_t array_length;
    struct regular_struct *struct_array[5];
} GTY((tag("ARRAY_STRUCT")));

/* TYPE_CALLBACK: Callback function pointer */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_struct {
    callback_func handler;
    void *user_data GTY((skip));
} GTY((tag("CALLBACK_STRUCT")));

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific {
    int lang_id;
    void *lang_data;
} GTY((lang_struct));

/* Complex nested type to ensure traversal */
struct container {
    struct regular_struct regular;
    struct user_defined user;
    union my_union union_val;
    struct pointer_struct pointers;
    struct array_struct arrays;
    struct callback_struct callback;
    struct lang_specific lang;
    struct string_struct string;
} GTY((tag("CONTAINER")));

/* Function pointer typedef */
typedef int (*comparator_func)(const void*, const void*) GTY((callback));

/* Another union with nested structs */
union complex_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        char *name GTY((length("strlen($1->name) + 1")));
        int value;
    } named_value;
} GTY((tag("COMPLEX_UNION")));

#endif /* TEST_TYPES_H */
