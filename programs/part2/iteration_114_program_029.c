#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* TYPE_SCALAR: Fundamental types with GTY annotation */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_UNDEFINED: Forward declaration that will never be defined */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Standard C struct */
struct my_struct GTY(())
{
    my_scalar_t field1;
    another_scalar_t field2;
    struct undefined_struct *forward_ptr;  /* Pointer to undefined type */
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user))
{
    int user_field1;
    char user_field2;
};

/* TYPE_UNION */
union my_union GTY(())
{
    int int_val;
    double double_val;
    struct my_struct *struct_ptr;
};

/* TYPE_ARRAY: Various array types */
struct array_container GTY(())
{
    int fixed_array[10] GTY(());
    int variable_array[] GTY(());
    struct my_struct *struct_array[5] GTY(());
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    const char *string_field GTY((length("strlen($1)")));
    char *another_string GTY((length("strlen($1)")));
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char *) GTY((callback));

struct callback_container GTY(())
{
    callback_func handler;
    void (*direct_callback)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_field1;
    void *lang_field2;
};

/* TYPE_POINTER: Complex pointer relationships */
struct pointer_network GTY(())
{
    struct my_struct *direct_ptr;
    struct pointer_network *self_ptr;  /* Self-referential */
    struct array_container *array_ptr;
    union my_union *union_ptr;
};

/* Nested struct for additional complexity */
struct outer_struct GTY(())
{
    struct inner_nested GTY(())
    {
        int nested_field;
        struct outer_struct *parent_ptr;
    } nested;
    
    struct
    {
        int anonymous_field;
    } anonymous;
    
    union
    {
        int anon_int;
        float anon_float;
    } anon_union;
};

/* Another forward declaration for TYPE_UNDEFINED coverage */
struct another_undefined GTY(());

/* Struct using the forward-declared type */
struct uses_undefined GTY(())
{
    struct another_undefined *undefined_ptr;
    struct undefined_struct *another_undefined_ptr;
};

/* Array with length attribute */
struct length_array_container GTY(())
{
    int count;
    int *dynamic_array GTY((length("$1.count")));
    struct my_struct **struct_ptr_array GTY((length("$1.count")));
};

#endif /* TEST_GENGTYPE_H */
