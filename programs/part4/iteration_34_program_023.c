#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_struct GTY(()) {
    char *data GTY((length("strlen($1->data) + 1")));
    int length;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    my_scalar value;
    struct plain_struct *next;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_struct GTY((user)) {
    void *data;
    size_t size;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    float float_val;
    char *string_val GTY((length("strlen($1.string_val) + 1")));
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(()) {
    struct plain_struct *plain_ptr;
    union my_union *union_ptr;
    void *opaque_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int *variable_array GTY((length("$1->var_len")));
    size_t var_len;
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*callback_type)(int, char *) GTY((callback));

struct callback_struct GTY(()) {
    callback_type handler;
    void *context;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int lang_specific_field;
    void *lang_data;
};

/* Complex nested type to ensure thorough traversal */
struct complex_type GTY(()) {
    /* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
    struct forward_decl *fwd_ptr;
    
    /* TYPE_STRUCT */
    struct plain_struct nested_struct;
    
    /* TYPE_UNION */
    union my_union nested_union;
    
    /* TYPE_POINTER */
    struct pointer_struct *ptr_struct;
    
    /* TYPE_ARRAY */
    struct array_struct arrays[5];
    
    /* TYPE_STRING */
    struct string_struct str_field;
    
    /* TYPE_CALLBACK */
    callback_type cb_field;
    
    /* TYPE_LANG_STRUCT pointer */
    struct lang_struct *lang_ptr;
};

/* Forward declaration for undefined type */
struct forward_decl GTY(());

/* Now define it to resolve the undefined type */
struct forward_decl GTY(()) {
    int value;
    struct complex_type *complex_ptr;
};

/* TYPE_UNDEFINED: Another undefined type (never defined) */
struct never_defined GTY(());

/* Array of pointers */
typedef struct plain_struct *ptr_array[20] GTY(());

/* Nested array in struct */
struct nested_array_struct GTY(()) {
    ptr_array pointers;
    int count;
};

/* Union containing struct */
union struct_union GTY(()) {
    struct plain_struct s;
    struct array_struct a;
};

/* Function pointer with attributes */
typedef int (*attr_callback)(const char *, ...) 
    __attribute__((format(printf, 1, 2))) 
    GTY((callback));

/* Struct with bitfields */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Anonymous struct/union */
struct anonymous_struct GTY(()) {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* Const-qualified pointer */
struct const_struct GTY(()) {
    const struct plain_struct *const_ptr;
    volatile int volatile_field;
};

/* Packed struct */
struct packed_struct GTY(()) {
    char a;
    int b;
    char c;
} __attribute__((packed));

#endif /* TEST_TYPES_H */
