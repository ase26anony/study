#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* Forward declarations for TYPE_UNDEFINED and circular references */
struct forward_declared_struct GTY(());
struct another_forward GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t id;
    struct forward_declared_struct *next;  /* TYPE_POINTER to undefined type */
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user)) {
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION */
union data_union GTY(()) {
    my_scalar_t as_scalar;
    char *as_string;
    struct base_struct *as_struct;
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    struct base_struct *ptr_array[5] GTY(());
    int variable_array[] GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char *name GTY((length("name_len")));
    int name_len;
    const char *constant_string GTY((length("strlen(constant_string)")));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler GTY(());
    void (*direct_callback)(struct base_struct*) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific GTY((lang_struct(1))) {
    int lang_data;
    void *lang_private;
};

/* Nested structures for complexity */
struct outer_struct GTY(()) {
    struct base_struct inner GTY(());
    union data_union choice GTY(());
    struct {
        int nested_data;
        struct outer_struct *parent GTY(());
    } nested GTY(());
};

/* Now define the forward-declared structs to complete TYPE_UNDEFINED -> TYPE_STRUCT */
struct forward_declared_struct GTY(()) {
    int data;
    struct outer_struct *owner GTY(());
    struct another_forward *peer GTY(());
};

struct another_forward GTY(()) {
    char tag;
    struct forward_declared_struct *link GTY(());
};

/* Additional pointer types for coverage */
typedef struct base_struct *base_ptr_t GTY(());
typedef union data_union *union_ptr_t GTY(());

#endif /* TEST_GENGYPE_H */
