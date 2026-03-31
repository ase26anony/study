#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C struct with GTY tag */
struct my_struct GTY(())
{
    my_scalar_t field1;
    another_scalar field2;
    struct undefined_struct* forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user))
{
    int user_data;
    void* user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(())
{
    my_scalar_t scalar_member;
    struct my_struct* struct_member;
    char* string_member;
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char zero_length_array[0] GTY(());
    
    /* Variable-length array with length attribute */
    struct my_struct* variable_array GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    char* regular_string GTY(());  /* Regular char pointer */
    char* counted_string GTY((length("str_len")));  /* String with length */
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char*) GTY((callback));

struct callback_container GTY(())
{
    callback_func handler;
    void (*regular_func_ptr)(void) GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void* lang_pointer;
};

/* Nested types for complexity */
struct outer_container GTY(())
{
    /* Nested struct */
    struct nested_struct GTY(())
    {
        int nested_data;
        struct my_struct* parent_ptr;
    } nested;
    
    /* Nested union */
    union nested_union GTY(())
    {
        int union_int;
        char union_char;
    } nested_union;
    
    /* Pointer to self for circular reference */
    struct outer_container* self_ptr;
    
    /* Array of pointers */
    struct my_struct* ptr_array[5] GTY(());
    
    /* Multi-dimensional array */
    int matrix[3][3] GTY(());
};

/* TYPE_POINTER: Various pointer types */
struct pointer_container GTY(())
{
    /* Simple pointer */
    struct my_struct* simple_ptr;
    
    /* Pointer to pointer */
    struct my_struct** double_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer */
    int (*func_ptr)(int, int);
    
    /* Pointer to union */
    union my_union* union_ptr;
    
    /* Circular reference */
    struct pointer_container* next;
};

/* Now define the previously undefined struct to create TYPE_UNDEFINED->TYPE_STRUCT transition */
struct undefined_struct GTY(())
{
    int finally_defined;
    struct my_struct* back_reference;
};

/* Recursive type definition */
struct recursive_node GTY(())
{
    int data;
    struct recursive_node* left;
    struct recursive_node* right;
    struct recursive_node* parent;
};

#endif /* TEST_GENGYPE_H */
