#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_struct GTY(()) {
    char * GTY((length("strlen($1)"))) data;
    int length;
};

/* TYPE_STRUCT: Regular struct */
struct regular_struct GTY(()) {
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with callback */
struct user_struct GTY((user)) {
    void *data;
    size_t size;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char * GTY((length("strlen($1)"))) str_val;
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(()) {
    struct regular_struct * GTY((skip)) ptr1;
    struct string_struct * GTY((skip)) ptr2;
    void (* GTY((callback)) callback_func)(void);
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int * GTY((length("$1->dynamic_length"))) dynamic_array;
    int dynamic_length;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_type GTY((callback)))(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct)) {
    int lang_field1;
    void *lang_field2;
};

/* Complex nested type to ensure thorough traversal */
struct complex_nested GTY(()) {
    struct regular_struct nested_struct;
    union my_union nested_union;
    struct array_struct nested_array;
    callback_type callback_field;
    struct lang_specific_struct *lang_struct_ptr;
};

/* Forward declarations for pointer cycles */
struct forward_declared GTY(());
struct another_forward GTY(());

struct forward_declared GTY(()) {
    struct another_forward *next;
    int value;
};

struct another_forward GTY(()) {
    struct forward_declared *prev;
    char * GTY((length("strlen($1)"))) name;
};

/* TYPE_UNDEFINED: This might be triggered by incomplete types */
/* Let's create an incomplete struct declaration */
struct incomplete_struct;

/* Variably modified type */
struct vm_struct GTY(()) {
    int n;
    int arr[];  /* Flexible array member */
};

/* Tagged union for more complexity */
typedef union tagged_union GTY((tag("type"))) {
    int type;
    struct {
        int type;
        int value;
    } int_val;
    struct {
        int type;
        double value;
    } double_val;
    struct {
        int type;
        char * GTY((length("strlen($1.value)"))) str;
    } string_val;
} tagged_union_t;

#endif /* TEST_TYPES_H */
