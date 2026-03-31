#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());
typedef char char_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED handling */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_base_struct GTY(())
{
    my_scalar_t field1;
    another_scalar_t field2;
};

/* Nested struct definition */
struct outer_struct GTY(())
{
    struct my_base_struct base;
    struct forward_declared_struct* forward_ptr;  /* TYPE_POINTER to undefined type */
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user))
{
    my_scalar_t user_field;
    char* user_string;
};

/* TYPE_UNION */
union my_union GTY(())
{
    my_scalar_t as_scalar;
    struct my_base_struct* as_struct_ptr;  /* TYPE_POINTER */
    char* as_string;                       /* TYPE_STRING (when used with length) */
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(())
{
    /* Fixed-size array */
    my_scalar_t fixed_array[10] GTY(());
    
    /* Zero-length array */
    char zero_length_array[0] GTY(());
    
    /* Variable-length array with length attribute */
    struct my_base_struct* variable_array GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    char* regular_string;                     /* Regular char pointer */
    char* counted_string GTY((length("str_len")));  /* TYPE_STRING */
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char*) GTY((callback));

struct callback_container GTY(())
{
    callback_func_t handler GTY(());
    void (*regular_func_ptr)(void);           /* Regular function pointer */
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_field1;
    void* lang_field2;
};

/* Complete the forward-declared struct definition */
struct forward_declared_struct GTY(())
{
    struct outer_struct* outer_ptr;           /* Circular reference */
    union my_union data;
};

/* Complex nested type with multiple interdependencies */
struct master_container GTY(())
{
    /* Nested union */
    union {
        struct my_base_struct nested_struct;
        struct forward_declared_struct* fwd_ptr;
    } nested_union GTY(());
    
    /* Array of pointers */
    struct forward_declared_struct* ptr_array[5] GTY(());
    
    /* Pointer to array */
    int (*matrix_ptr)[10] GTY(());
    
    /* String array */
    char* string_array[3] GTY(());
    
    /* Callback field */
    callback_func_t callbacks[2] GTY(());
};

/* Additional pointer types for coverage */
typedef struct my_base_struct* base_ptr_t GTY(());
typedef union my_union* union_ptr_t GTY(());

#endif /* TEST_GENGYPE_H */
