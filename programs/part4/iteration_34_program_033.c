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

/* TYPE_STRUCT: Plain struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with marker */
struct user_struct GTY((user)) {
    void *data;
    size_t size;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int i;
    double d;
    char * GTY((tag("0"))) s;
    struct plain_struct * GTY((tag("1"))) ps;
};

/* TYPE_POINTER: Various pointer types */
struct pointer_struct GTY(()) {
    struct plain_struct * GTY((skip)) ptr1;
    union my_union *ptr2;
    struct string_struct ** GTY((chain)) ptr3;
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int * GTY((length("$1->dyn_length"))) dyn_array;
    size_t dyn_length;
    
    /* Nested array in struct */
    struct {
        char * GTY((length("5"))) small_array[5];
    } nested;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void * GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int lang_specific_field;
    void * GTY((skip)) lang_data;
};

/* Complex nested type to ensure thorough traversal */
struct master_container GTY(()) {
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    struct user_struct *user_struct_ptr;
    union my_union union_field;
    struct pointer_struct *pointer_field;
    struct array_struct array_field;
    struct callback_container callback_field;
    struct lang_struct *lang_field;
    
    /* Self-referential pointer */
    struct master_container * GTY((skip)) next;
};

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct;

/* Function using undefined type */
void use_undefined(struct undefined_struct * GTY((skip)) undef_ptr);

#endif /* TEST_TYPES_H */
