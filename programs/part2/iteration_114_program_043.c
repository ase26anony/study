#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct GTY(());
typedef struct forward_declared_struct *forward_ptr_t GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRING: String type with length attribute */
struct string_container {
    char * GTY((length("strlen($1) + 1"))) data;
    int length;
} GTY(());

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1))) {
    int lang_field;
    void *lang_data;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_managed GTY((user)) {
    int user_id;
    char *user_name;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_container GTY(()) {
    int as_int;
    float as_float;
    char * GTY((tag("0"))) as_string;
    struct forward_declared_struct *as_ptr;
};

/* TYPE_ARRAY: Various array types */
struct array_examples GTY(()) {
    /* Fixed-size array */
    int fixed_array[10] GTY(());

    /* Zero-length array */
    char flexible_array[0] GTY(());

    /* Array with length attribute */
    int * GTY((length("$1->array_length"))) dynamic_array;
    
    /* Nested array in struct */
    struct {
        int matrix[3][3];
    } nested;
    
    int array_length;
};

/* TYPE_STRUCT: Regular struct with multiple fields */
struct regular_struct GTY(()) {
    my_scalar_t scalar_field;
    struct string_container *string_field;
    union data_container union_field;
    callback_func_t callback_field;
    struct array_examples array_field;
    
    /* Nested struct */
    struct {
        int inner_field;
        char inner_char;
    } nested_struct;
    
    /* Pointer to self for recursion */
    struct regular_struct *next GTY(());
    
    /* Pointer to forward-declared type */
    forward_ptr_t forward_ref;
};

/* TYPE_POINTER: Struct focusing on pointer relationships */
struct pointer_network GTY(()) {
    /* Simple pointer */
    struct regular_struct *regular_ptr GTY(());
    
    /* Pointer to union */
    union data_container *union_ptr GTY(());
    
    /* Pointer to array struct */
    struct array_examples *array_ptr GTY(());
    
    /* Pointer to callback type */
    callback_func_t *callback_ptr GTY(());
    
    /* Circular reference */
    struct pointer_network *peer GTY(());
    
    /* Pointer to lang struct */
    struct lang_specific *lang_ptr GTY(());
    
    /* Pointer to user struct */
    struct user_managed *user_ptr GTY(());
    
    /* Double pointer */
    struct regular_struct **double_ptr GTY(());
};

/* Now define the forward-declared struct to resolve TYPE_UNDEFINED */
struct forward_declared_struct GTY(()) {
    int id;
    char name[32];
    struct pointer_network *network_ref GTY(());
};

/* Additional complex nested type */
struct outer_container GTY(()) {
    /* Union containing struct */
    union {
        struct regular_struct regular;
        struct pointer_network network;
    } data_union;
    
    /* Array of structs */
    struct regular_struct struct_array[5] GTY(());
    
    /* Pointer array */
    struct pointer_network *ptr_array[3] GTY(());
    
    /* Nested anonymous struct */
    struct {
        int depth;
        struct {
            int inner_depth;
            char label[16];
        } deeper;
    } hierarchy;
};

#endif /* TEST_GENGYPE_H */
