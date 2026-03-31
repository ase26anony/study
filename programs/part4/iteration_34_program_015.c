#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar typedef */
typedef int my_scalar GTY(());
typedef unsigned long my_other_scalar GTY(());

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(int, void*) GTY((callback));

/* TYPE_POINTER: Pointer type */
typedef struct my_struct* my_struct_ptr GTY(());

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Plain struct */
struct my_struct GTY(()) {
    int field1;
    double field2;
    struct my_struct* next GTY(());
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_struct GTY((user)) {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union type */
union my_union GTY(()) {
    int int_val;
    double double_val;
    char* str_val GTY(());
    struct my_struct* struct_ptr GTY(());
};

/* TYPE_ARRAY: Struct with array fields */
struct array_container GTY(()) {
    int fixed_array[10];
    int* variable_array GTY((length("var_len")));
    size_t var_len;
    
    /* Nested array in struct */
    struct {
        char* strings[5] GTY(());
    } nested;
};

/* TYPE_STRING: String handling */
struct string_container GTY(()) {
    /* String with length attribute */
    char* dynamic_string GTY((length("str_len")));
    size_t str_len;
    
    /* Fixed string array */
    char fixed_string[100];
    
    /* Pointer to string */
    const char* const_string GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct)) {
    int lang_data;
    void* lang_pointer GTY(());
    
    /* Nested language-specific type */
    struct {
        int nested_lang_field;
    } GTY((lang_struct)) inner;
};

/* Complex type combining multiple kinds */
struct complex_type GTY(()) {
    /* Scalar */
    my_scalar scalar_field;
    
    /* Pointer */
    my_struct_ptr struct_pointer;
    
    /* Array */
    struct array_container arrays[3];
    
    /* Union */
    union my_union data_union;
    
    /* String */
    struct string_container strings;
    
    /* Callback */
    callback_func handler GTY(());
    
    /* Nested struct */
    struct {
        int nested_int;
        char* nested_string GTY(());
    } nested_struct;
    
    /* Pointer to language struct */
    struct lang_specific* lang_ptr GTY(());
};

/* Another union with GTY-marked fields */
union tagged_union GTY(()) {
    struct my_struct* s GTY(());
    struct array_container* a GTY(());
    struct string_container* str GTY(());
    callback_func cb GTY(());
};

/* Type with variable-length array at end (GCC extension) */
struct vla_struct GTY(()) {
    int count;
    int data[];  /* Flexible array member */
};

/* Opaque pointer type */
typedef void* opaque_handle GTY(());

/* Enum type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Struct with bitfields */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 5;
    int regular_field;
};

/* Self-referential structure */
struct tree_node GTY(()) {
    int value;
    struct tree_node* left GTY(());
    struct tree_node* right GTY(());
    struct tree_node* parent GTY(());
};

/* Array of pointers */
typedef struct my_struct* struct_ptr_array[10] GTY(());

/* Function pointer with complex signature */
typedef int (*complex_callback)(
    struct my_struct* GTY(()),
    struct array_container* GTY(()),
    callback_func GTY(())
) GTY((callback));

/* Container with all type kinds */
struct master_container GTY(()) {
    /* TYPE_SCALAR */
    my_scalar scalar_member;
    my_enum enum_member;
    
    /* TYPE_POINTER */
    struct my_struct* struct_ptr_member GTY(());
    opaque_handle handle_member;
    
    /* TYPE_STRUCT */
    struct my_struct struct_member;
    
    /* TYPE_USER_STRUCT */
    struct user_struct user_member;
    
    /* TYPE_UNION */
    union my_union union_member;
    
    /* TYPE_ARRAY */
    struct array_container array_member;
    int int_array[20];
    
    /* TYPE_STRING */
    struct string_container string_member;
    char* raw_string GTY(());
    
    /* TYPE_CALLBACK */
    callback_func callback_member GTY(());
    complex_callback complex_callback_member GTY(());
    
    /* TYPE_LANG_STRUCT */
    struct lang_specific* lang_ptr_member GTY(());
    
    /* Nested container */
    struct {
        int depth;
        struct master_container* next GTY(());
    } nested_container;
    
    /* Union with tag */
    union {
        int as_int;
        double as_double;
        struct my_struct* as_struct GTY(());
    } tagged_union_member;
    
    /* Pointer array */
    struct_ptr_array ptr_array;
    
    /* Variable length array pointer */
    struct vla_struct* vla_ptr GTY(());
    
    /* Bitfield struct */
    struct bitfield_struct bitfield_member;
    
    /* Tree structure */
    struct tree_node* tree_root GTY(());
    
    /* Union of pointers */
    union tagged_union poly_union;
};

/* Global variable declarations to force type instantiation */
extern struct master_container GTY(()) global_container;
extern struct lang_specific GTY(()) global_lang_struct;
extern union my_union GTY(()) global_union;

#endif /* TEST_TYPES_H */
