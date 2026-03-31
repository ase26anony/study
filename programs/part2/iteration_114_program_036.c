#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct my_struct GTY(())
{
    my_scalar_t field1;
    another_scalar_t field2;
    struct my_struct *next GTY((skip)); /* Pointer field */
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user))
{
    int user_data;
    void *user_pointer;
};

/* TYPE_UNION: Union with GTY-tagged members */
union my_union GTY(())
{
    my_scalar_t scalar_val;
    struct my_struct *struct_ptr GTY((skip));
    char *string_ptr;
};

/* TYPE_POINTER: Struct containing various pointers */
struct pointer_container GTY(())
{
    struct my_struct *direct_ptr GTY((skip));
    struct undefined_struct *undefined_ptr GTY((skip)); /* TYPE_UNDEFINED pointer */
    union my_union *union_ptr GTY((skip));
    struct pointer_container *self_ptr GTY((skip)); /* Recursive pointer */
};

/* TYPE_ARRAY: Structs containing arrays */
struct array_container GTY(())
{
    int fixed_array[10];
    struct my_struct *ptr_array[5] GTY((skip));
    int zero_length_array[0]; /* Zero-length array */
    struct my_struct *variable_array GTY((length("array_length")));
    int array_length;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    char *regular_string GTY((length("str_len")));
    const char *const_string GTY((length("const_str_len")));
    int str_len;
    int const_str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(())
{
    callback_func_t handler;
    void (*another_handler)(struct my_struct*) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_pointer;
};

/* Nested types for complex interdependencies */
struct outer_container GTY(())
{
    /* Nested struct */
    struct nested_struct GTY(())
    {
        int nested_data;
        struct outer_container *parent_ptr GTY((skip));
    } nested;
    
    /* Nested union */
    union nested_union GTY(())
    {
        int union_int;
        struct nested_struct *nested_ptr GTY((skip));
    } nested_union;
    
    /* Array of nested types */
    struct nested_struct nested_array[3];
    
    /* Pointer to forward-declared type */
    struct forward_declared *forward_ptr GTY((skip));
};

/* Forward declaration for circular reference */
struct forward_declared GTY(());

/* Now define the forward-declared struct */
struct forward_declared GTY(())
{
    struct outer_container *outer_ptr GTY((skip));
    struct forward_declared *next GTY((skip));
};

/* Additional complex type with multiple dependencies */
struct complex_type GTY(())
{
    /* Mix of all type kinds */
    my_scalar_t scalar_field;
    struct my_struct struct_field;
    union my_union union_field;
    struct pointer_container *ptr_field GTY((skip));
    struct array_container array_field;
    struct string_container string_field;
    struct callback_container callback_field;
    struct lang_specific_struct *lang_field GTY((skip));
    
    /* Self-reference for circular dependency */
    struct complex_type *next GTY((skip));
    struct complex_type *prev GTY((skip));
};

#endif /* TEST_GENGYPE_H */
