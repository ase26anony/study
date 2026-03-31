#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(()) {
    my_scalar_t value;
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user)) {
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(()) {
    my_scalar_t scalar_val;
    struct base_struct* struct_ptr;
    char* string_ptr;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(()) {
    struct base_struct* direct_ptr;
    struct undefined_struct* undefined_ptr;
    union my_union* union_ptr;
    struct pointer_container* self_ptr;  /* Recursive pointer */
};

/* TYPE_ARRAY: Structs with arrays */
struct array_container GTY(()) {
    /* Fixed-size array */
    struct base_struct* fixed_array[10] GTY(());
    
    /* Zero-length array */
    int zero_length_array[0] GTY(());
    
    /* Variable-length array with length attribute */
    struct pointer_container** var_array GTY((length("var_length")));
    int var_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(()) {
    char* regular_string;
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(()) {
    callback_func_t handler;
    void* user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1))) {
    int lang_data;
    void* lang_pointer;
};

/* Nested struct definition (creates more complex type graph) */
struct outer_container GTY(()) {
    struct {
        int nested_data;
        struct base_struct* nested_ptr;
    } GTY(())) inner_struct;
    
    union {
        my_scalar_t u1;
        struct array_container* u2;
    } GTY(())) inner_union;
    
    struct outer_container* next;  /* Linked list pointer */
};

/* Another forward declaration for circular reference */
struct circular_a GTY(());
struct circular_b GTY(());

struct circular_a GTY(()) {
    struct circular_b* b_ptr;
    int data_a;
};

struct circular_b GTY(()) {
    struct circular_a* a_ptr;
    int data_b;
};

/* Array of pointers with nested struct */
struct complex_array GTY(()) {
    struct {
        int tag;
        union {
            struct base_struct* s_ptr;
            struct array_container* a_ptr;
        } GTY(())) data;
    } GTY(())) elements[5];
    
    callback_func_t callbacks[3];
};

#endif /* TEST_GENGYPE_H */
