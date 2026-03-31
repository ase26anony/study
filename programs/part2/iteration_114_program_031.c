#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED (incomplete type) */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(()) {
    my_scalar_t field1;
    another_scalar_t field2;
    struct forward_declared_struct *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_field1;
    char user_field2;
};

/* TYPE_UNION */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct my_struct *as_struct_ptr;
    char *as_string;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    int variable_array[] GTY(());
    struct my_struct *ptr_array[5] GTY(());
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char *regular_string GTY((length("strlen($) + 1")));
    const char *const_string GTY((length("strlen($) + 1")));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*another_handler)(struct my_struct *) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_field1;
    void *lang_field2;
};

/* Nested types for complexity */
struct outer_struct GTY(()) {
    struct {
        int nested_field1;
        union {
            int nested_union_field1;
            char *nested_union_field2;
        } nested_union GTY(());
    } nested_struct GTY(());
    
    struct array_container *array_ptr;
    union my_union union_field;
};

/* Circular references */
struct node_a GTY(()) {
    int value;
    struct node_b *next_b;
};

struct node_b GTY(()) {
    int value;
    struct node_a *next_a;
    struct node_b *self_ptr;  /* Self-referential */
};

/* Now define the forward-declared struct (was TYPE_UNDEFINED) */
struct forward_declared_struct GTY(()) {
    int defined_field;
    struct my_struct *back_ref;
};

/* TYPE_POINTER: Various pointer types */
typedef struct my_struct *my_struct_ptr_t GTY(());
typedef union my_union *my_union_ptr_t GTY(());

/* Array of pointers with length attribute */
struct pointer_array GTY(()) {
    struct my_struct **ptr_list GTY((length("$->count")));
    int count;
};

#endif /* TEST_GENGYPE_H */
