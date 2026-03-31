#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(()) {
    my_scalar_t field1;
    another_scalar field2;
    struct my_struct *next GTY((skip));  /* TYPE_POINTER inside struct */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    my_scalar_t as_scalar;
    struct my_struct *as_struct_ptr GTY((skip));
    char *as_string GTY((length));
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(()) {
    int fixed_array[10] GTY(());
    int variable_array[] GTY(());
    struct my_struct *ptr_array[5] GTY((skip));
    int (*callback_array[3])(void) GTY((callback));
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char *regular_string GTY((length));
    const char *const_string GTY((length));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef int (*callback_func)(int, char *) GTY((callback));

struct callback_container GTY(()) {
    callback_func handler;
    void (*raw_callback)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void *lang_ptr;
};

/* Complex nested types for interdependencies */
struct complex_node GTY(()) {
    int data;
    
    /* TYPE_POINTER: Pointer to forward-declared type */
    struct undefined_struct *undefined_ptr GTY((skip));
    
    /* TYPE_POINTER: Pointer to same type (recursive) */
    struct complex_node *next GTY((skip));
    
    /* TYPE_POINTER: Pointer to union */
    union my_union *union_ptr GTY((skip));
    
    /* TYPE_ARRAY: Array of pointers */
    struct my_struct *struct_array[3] GTY((skip));
    
    /* TYPE_STRING */
    char *name GTY((length));
    
    /* TYPE_CALLBACK */
    callback_func processor GTY((callback));
    
    /* Nested union */
    union {
        int nested_int;
        char *nested_string GTY((length));
    } nested_union GTY(());
    
    /* Nested struct */
    struct {
        int x;
        int y;
    } nested_struct GTY(());
};

/* Provide definition for forward-declared struct (after its use) */
struct undefined_struct GTY(()) {
    int finally_defined;
    struct complex_node *node_ptr GTY((skip));
};

/* Global variable declarations for instantiation */
extern struct my_struct global_struct_instance GTY(());
extern union my_union global_union_instance GTY(());
extern struct array_container global_array_instance GTY(());
extern struct string_container global_string_instance GTY(());
extern struct callback_container global_callback_instance GTY(());
extern struct lang_specific_struct global_lang_struct_instance GTY(());
extern struct complex_node global_complex_instance GTY(());

#endif /* TEST_GENGYPE_H */
