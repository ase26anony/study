#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY((user));
typedef int another_scalar_t GTY(());
typedef char char_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(())
{
    my_scalar_t field1;
    another_scalar_t field2;
    char_scalar_t field3;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user))
{
    int user_field1;
    float user_field2;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(())
{
    my_scalar_t as_scalar;
    struct base_struct * GTY((skip)) as_struct_ptr;
    char * GTY((length)) as_string;
};

/* TYPE_POINTER: Forward declaration for circular reference */
struct node_struct GTY(());

/* TYPE_ARRAY: Struct containing various array types */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char flexible_array[] GTY(());
    
    /* Array with length attribute */
    struct base_struct * GTY((length("array_len"))) dyn_array;
    int array_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    char * GTY((length("str_len"))) string_field;
    int str_len;
    
    const char * GTY((length)) const_string;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, float) GTY((callback));

struct callback_container GTY(())
{
    callback_func_t handler GTY(());
    void (* GTY((callback)) another_handler)(struct base_struct *);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_field1;
    void * GTY((skip)) lang_field2;
};

/* Complete definition of forward-declared struct for TYPE_POINTER */
struct node_struct GTY(())
{
    int data;
    struct node_struct * GTY((skip)) next;  /* Recursive pointer */
    struct undefined_struct * GTY((skip)) undefined_ptr;  /* Pointer to undefined type */
    union my_union node_union;
};

/* Nested struct definition */
struct outer_container GTY(())
{
    struct
    {
        int nested_field1;
        struct node_struct * GTY((skip)) nested_ptr;
    } inner_struct GTY(());
    
    union
    {
        int union_field1;
        double union_field2;
    } inner_union GTY(());
    
    struct array_container arrays;
    struct string_container strings;
};

/* Another struct with multiple pointer types */
struct pointer_network GTY(())
{
    struct node_struct * GTY((skip)) node_ptr;
    struct base_struct ** GTY((skip)) struct_ptr_ptr;
    void * GTY((skip)) void_ptr;
    const char * GTY((length)) const_char_ptr;
};

#endif /* TEST_GENGYPE_H */
