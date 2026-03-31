#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_UNDEFINED: Forward declaration (incomplete type) */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Standard C struct */
struct base_struct GTY(())
{
    my_scalar_t value;
    another_scalar_t count;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user))
{
    int data;
    void* extra;
};

/* TYPE_UNION */
union data_union GTY(())
{
    int int_val;
    float float_val;
    char* char_ptr;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(())
{
    /* Pointer to scalar */
    my_scalar_t* scalar_ptr GTY(());
    
    /* Pointer to struct */
    struct base_struct* struct_ptr GTY(());
    
    /* Pointer to forward-declared struct (TYPE_UNDEFINED initially) */
    struct undefined_struct* undefined_ptr GTY(());
    
    /* Self-referential pointer */
    struct pointer_container* next GTY(());
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char flexible_array[] GTY(());
    
    /* Array with length attribute */
    struct base_struct* variable_array GTY((length("len")));
    int len;
};

/* TYPE_STRING: String type */
struct string_container GTY(())
{
    /* String with length attribute */
    char* dynamic_string GTY((length("str_len")));
    int str_len;
    
    const char* const_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, void*) GTY((callback));

struct callback_container GTY(())
{
    callback_func handler GTY(());
    void* user_data GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1)))
{
    int lang_data;
    void* lang_ptr;
};

/* Nested types for complexity */
struct outer_container GTY(())
{
    /* Nested struct */
    struct inner_struct GTY(())
    {
        int inner_data;
        struct outer_container* parent GTY(());
    } inner;
    
    /* Nested union */
    union nested_union GTY(())
    {
        int option_a;
        float option_b;
        struct inner_struct* option_c GTY(());
    } choice;
    
    /* Array of pointers */
    struct inner_struct* ptr_array[5] GTY(());
};

/* Now define the previously undefined struct */
struct undefined_struct GTY(())
{
    int defined_now;
    struct pointer_container* back_ref GTY(());
    struct outer_container* complex_ref GTY(());
};

/* Complex circular reference structure */
struct node GTY(())
{
    int id;
    struct node* left GTY(());
    struct node* right GTY(());
    struct node** neighbors GTY((length("neighbor_count")));
    int neighbor_count;
};

/* Mixed type container */
struct mixed_container GTY(())
{
    /* All type kinds in one struct */
    my_scalar_t scalar_field;
    struct base_struct struct_field;
    union data_union union_field;
    struct base_struct* pointer_field GTY(());
    int array_field[5] GTY(());
    char* string_field GTY((length("string_len")));
    int string_len;
    callback_func callback_field GTY(());
    struct lang_specific_struct* lang_field GTY(());
};

#endif /* TEST_GENGTYPE_H */
