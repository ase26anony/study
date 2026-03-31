#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY(());

/* TYPE_STRUCT: Standard C struct with GTY tag */
struct my_struct GTY(())
{
    my_scalar_t scalar_field;
    int another_field;
    struct undefined_struct *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct marked for special user handling */
struct user_handled_struct GTY((user))
{
    int data;
    char *description;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(())
{
    my_scalar_t as_scalar;
    struct my_struct *as_struct_ptr;
    char *as_string;
};

/* TYPE_ARRAY: Helper struct for array length */
struct array_info GTY(())
{
    int length;
};

/* TYPE_STRING: String type using GTY((length)) */
struct string_container GTY(())
{
    char * GTY((length("str_len"))) string_field;
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_pointer;
};

/* Complex nested structure with multiple type kinds */
struct container_struct GTY(())
{
    /* TYPE_POINTER: Various pointer types */
    struct my_struct *struct_ptr GTY(());
    union my_union *union_ptr GTY(());
    
    /* TYPE_ARRAY: Fixed-size array */
    int fixed_array[10];
    
    /* TYPE_ARRAY: Zero-length array */
    char flexible_array GTY((length("flex_len"))) [];
    int flex_len;
    
    /* TYPE_ARRAY: Array with length attribute */
    struct my_struct * GTY((length("array_len"))) struct_array[];
    int array_len;
    
    /* Nested union */
    union {
        int nested_int;
        char *nested_string GTY((length("nested_str_len")));
        int nested_str_len;
    } nested_union GTY(());
    
    /* TYPE_CALLBACK: Callback field */
    callback_func_t callback GTY(());
    
    /* Pointer to lang struct */
    struct lang_specific_struct *lang_ptr GTY(());
    
    /* User struct instance */
    struct user_handled_struct user_struct;
};

/* Circular reference structure */
struct node GTY(())
{
    int value;
    struct node *next GTY(());
    struct node *prev GTY(());
};

/* Another structure with array of pointers */
struct graph GTY(())
{
    int node_count;
    struct node * GTY((length("node_count"))) nodes[];
};

#endif /* TEST_GENGYPE_H */
