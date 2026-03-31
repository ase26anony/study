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
    my_scalar_t field1;
    another_scalar field2;
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
    my_scalar_t scalar_val;
    struct base_struct *struct_ptr GTY(());
    char *string_data GTY((length));
};

/* Forward declaration for circular reference */
struct forward_declared GTY(());

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(())
{
    /* Pointer to scalar */
    my_scalar_t *scalar_ptr GTY(());
    
    /* Pointer to struct */
    struct base_struct *base_ptr GTY(());
    
    /* Pointer to forward-declared struct (creates TYPE_UNDEFINED initially) */
    struct forward_declared *forward_ptr GTY(());
    
    /* Self-referential pointer */
    struct pointer_container *self_ptr GTY(());
    
    /* Pointer to union */
    union data_union *union_ptr GTY(());
};

/* TYPE_ARRAY: Struct containing arrays */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char zero_length_array[0] GTY(());
    
    /* Array with length attribute */
    struct base_struct *variable_array GTY((length("var_len")));
    int var_len;
    
    /* Array of pointers */
    struct pointer_container *ptr_array[5] GTY(());
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    /* String field */
    char *string_field GTY((length("str_len")));
    int str_len;
    
    /* Multiple strings */
    const char *const_string GTY((length));
    char *another_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char *) GTY((callback));

struct callback_container GTY(())
{
    callback_func handler GTY(());
    
    /* Another callback directly in struct */
    int (*direct_callback)(struct base_struct *, union data_union *) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_pointer;
};

/* Now define the forward-declared struct to complete the graph */
struct forward_declared GTY(())
{
    struct pointer_container *container GTY(());
    struct array_container arrays GTY(());
    union data_union data GTY(());
};

/* Nested types for additional complexity */
struct outer_container GTY(())
{
    struct inner_nested GTY(())
    {
        int nested_data;
        struct outer_container *parent_ptr GTY(());
    } nested GTY(());
    
    union inner_union GTY(())
    {
        int union_int;
        double union_double;
    } u GTY(());
};

/* Global type definitions for instantiation */
typedef struct base_struct base_t GTY(());
typedef union data_union data_u GTY(());
typedef struct array_container array_c GTY(());

#endif /* TEST_GENGTYPE_H */
