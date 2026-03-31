#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY(());
typedef struct undefined_struct *undefined_ptr_t GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(())
{
    my_scalar_t id;
    char *name GTY((length));
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled GTY((user))
{
    int user_data;
    void *user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(())
{
    my_scalar_t as_scalar;
    struct base_struct *as_struct GTY(());
    char *as_string GTY((length));
};

/* Forward declaration for circular reference */
struct forward_declared GTY(());

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_struct GTY(())
{
    /* Pointer to scalar */
    my_scalar_t *scalar_ptr GTY(());
    
    /* Pointer to struct */
    struct base_struct *struct_ptr GTY(());
    
    /* Pointer to forward-declared struct (creates TYPE_UNDEFINED initially) */
    struct forward_declared *forward_ptr GTY(());
    
    /* Self-referential pointer */
    struct pointer_struct *self_ptr GTY(());
    
    /* Pointer to undefined type */
    undefined_ptr_t undefined_ptr GTY(());
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_struct GTY(())
{
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char zero_length[] GTY(());
    
    /* Array with length attribute */
    struct base_struct *variable_array GTY((length ("array_len")));
    int array_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char *) GTY((callback));

struct callback_struct GTY(())
{
    callback_func handler GTY(());
    void (*plain_func_ptr)(void) GTY(());
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_pointer;
};

/* Nested struct definition */
struct container GTY(())
{
    /* Nested struct */
    struct nested GTY(())
    {
        int nested_data;
        struct base_struct *nested_ptr GTY(());
    } inner;
    
    /* Nested union */
    union nested_union GTY(())
    {
        int union_data;
        char *union_string GTY((length));
    } u_inner;
    
    /* Array of structs */
    struct base_struct struct_array[5] GTY(());
};

/* Now define the forward-declared struct */
struct forward_declared GTY(())
{
    int forward_data;
    struct pointer_struct *back_ptr GTY(());
};

/* TYPE_STRING: Explicit string type usage */
struct string_container GTY(())
{
    /* String with length attribute */
    char *dynamic_string GTY((length ("str_len")));
    int str_len;
    
    /* Plain char pointer (not necessarily a string in GTY sense) */
    char *plain_char_ptr GTY(());
};

/* Complex recursive type structure */
struct node GTY(())
{
    int value;
    struct node *left GTY(());
    struct node *right GTY(());
    struct node **children GTY((length ("child_count")));
    int child_count;
};

#endif /* TEST_GENGTYPE_H */
