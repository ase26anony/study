#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length callback */
struct string_struct GTY(()) {
    char *data GTY((length("strlen($1->data) + 1")));
    int length;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(()) {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom markers */
struct user_struct GTY((user)) {
    void *data;
    size_t size;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int as_int;
    double as_double;
    void *as_ptr;
};

/* TYPE_POINTER: Pointer type in a struct */
struct pointer_struct GTY(()) {
    struct plain_struct *next GTY(());
    struct string_struct *str GTY(());
    void *generic_ptr GTY((skip));
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(()) {
    int fixed_array[10];
    int *variable_array GTY((length("$1->var_len")));
    size_t var_len;
};

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_struct GTY(()) {
    callback_func handler;
    void *context;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_struct GTY((lang_struct)) {
    int lang_specific_field;
    void *lang_data;
};

/* Complex nested type to ensure traversal */
struct complex_nested GTY(()) {
    struct plain_struct base;
    union my_union variant;
    struct array_struct arrays;
    struct pointer_struct *pointers GTY(());
    struct lang_struct lang_info;
};

/* Forward declarations for pointer cycles */
struct forward_decl GTY(());
struct another_forward GTY(());

struct forward_decl GTY(()) {
    int id;
    struct another_forward *link GTY(());
};

struct another_forward GTY(()) {
    char *name GTY((length("strlen($1->name) + 1")));
    struct forward_decl *backlink GTY(());
};

/* TYPE_UNDEFINED: Incomplete/undefined type */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr GTY(());

/* Enum type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Variable length struct with trailing array */
struct vla_struct GTY(()) {
    int count;
    int items[0] GTY((length("$1->count")));
};

/* Function pointer in struct */
struct funcptr_struct GTY(()) {
    int (*compare)(const void*, const void*);
    void (*free_func)(void*);
};

/* Anonymous union in struct */
struct anon_union_struct GTY(()) {
    int type;
    union {
        int int_val;
        double double_val;
        char *string_val GTY((length("strlen($1->string_val) + 1")));
    } value;
};

/* Const qualified pointer */
struct const_struct GTY(()) {
    const int *const_ptr;
    volatile char *volatile_ptr;
};

#endif /* TEST_TYPES_H */
