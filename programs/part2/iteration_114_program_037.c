#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());
typedef char char_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED (incomplete type) */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t id;
    char_scalar_t flag;
    struct forward_declared_struct* next;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_handle;
};

/* TYPE_UNION */
union variant_union GTY(()) {
    my_scalar_t as_scalar;
    char* as_string;
    struct base_struct* as_struct;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(()) {
    struct base_struct* struct_ptr;
    union variant_union* union_ptr;
    struct pointer_container* self_ptr;  /* Recursive pointer */
    struct pointer_container* next;      /* Circular reference */
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(()) {
    int fixed_array[10];
    struct base_struct* ptr_array[5];
    int flexible_array[];  /* Zero-length array */
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string;
    char* counted_string GTY((length("strlen($) + 1")));
    const char* const_string;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void (*raw_func_ptr)(void);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void* lang_extra;
};

/* Nested types for complexity */
struct outer_container GTY(()) {
    struct {
        int nested_id;
        char nested_data[20];
    } inner_struct;
    
    union {
        int as_int;
        double as_double;
    } inner_union;
    
    struct array_container arrays;
    struct string_container strings;
};

/* Now define the forward-declared struct (completes TYPE_UNDEFINED) */
struct forward_declared_struct GTY(()) {
    int data;
    struct base_struct* back_ref;
    struct pointer_container* container;
};

/* Global variable declarations */
extern struct base_struct global_base;
extern union variant_union global_union;
extern struct pointer_container global_pointer_container;
extern struct array_container global_array_container;
extern struct string_container global_string_container;
extern struct callback_container global_callback_container;
extern struct lang_specific_struct global_lang_struct;
extern struct outer_container global_outer_container;

#endif /* TEST_GENGYPE_H */
