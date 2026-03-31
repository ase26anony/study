#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_UNDEFINED: Forward declaration that will remain undefined */
struct undefined_struct GTY(());

/* Forward declaration for circular references */
struct forward_declared GTY(());

/* TYPE_STRUCT: Standard C struct with GTY tag */
struct my_struct GTY(())
{
    /* TYPE_SCALAR field */
    my_scalar_t scalar_field;
    
    /* TYPE_POINTER field to another struct */
    struct forward_declared *ptr_field;
    
    /* TYPE_POINTER field to undefined type */
    struct undefined_struct *undefined_ptr;
    
    /* TYPE_ARRAY field (fixed size) */
    int fixed_array[10];
    
    /* TYPE_ARRAY field with GTY length attribute */
    int *variable_array GTY((length("var_len")));
    int var_len;
    
    /* TYPE_STRING field with GTY length attribute */
    char *string_field GTY((length("str_len")));
    int str_len;
    
    /* TYPE_CALLBACK: Function pointer with callback attribute */
    void (*callback_func)(int) GTY((callback));
    
    /* Nested TYPE_UNION */
    union {
        int nested_int;
        float nested_float;
        struct forward_declared *nested_ptr;
    } nested_union;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user))
{
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(())
{
    int union_int;
    float union_float;
    double union_double;
    struct my_struct *union_struct_ptr;
    char *union_string GTY((length("union_str_len")));
    int union_str_len;
};

/* TYPE_ARRAY: Array-specific struct */
struct array_container GTY(())
{
    /* Zero-length array */
    int zero_length_array[0];
    
    /* Pointer to array */
    int (*array_ptr)[5];
    
    /* Multi-dimensional array */
    int matrix[3][4];
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_pointer;
};

/* TYPE_CALLBACK: Typedef for function pointer with callback attribute */
typedef int (*callback_type)(int, float) GTY((callback));

/* Complete the forward declaration */
struct forward_declared GTY(())
{
    int data;
    struct my_struct *back_ptr;  /* Circular reference */
    struct forward_declared *next;  /* Linked list */
};

/* Another struct with complex nested types */
struct complex_nested GTY(())
{
    /* Nested anonymous struct */
    struct {
        int inner_data;
        char *inner_string GTY((length("inner_len")));
        int inner_len;
    } anonymous_struct;
    
    /* Nested anonymous union */
    union {
        long long_val;
        double double_val;
    } anonymous_union;
    
    /* Array of pointers */
    struct my_struct *ptr_array[5];
    
    /* Pointer to array of pointers */
    struct forward_declared **ptr_to_ptr_array;
    
    /* Callback field */
    callback_type cb_field;
};

#endif /* TEST_GENGYPE_H */
