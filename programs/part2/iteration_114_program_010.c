#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY annotation */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* Forward declaration for TYPE_UNDEFINED and pointer references */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C struct with GTY tag */
struct my_struct GTY(())
{
    my_scalar_t scalar_field;
    struct forward_declared_struct *forward_ptr;
    int regular_int;
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
    my_scalar_t as_scalar;
    struct my_struct *as_struct_ptr;
    char *as_string;
};

/* TYPE_ARRAY: Struct containing various array types */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10] GTY(());
    
    /* Zero-length array */
    char flexible_array[] GTY(());
    
    /* Array with length attribute */
    struct my_struct *variable_array GTY((length("array_len")));
    int array_len;
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    char *regular_string GTY((length("str_len")));
    int str_len;
    
    const char *const_string GTY((length("const_len")));
    int const_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_container GTY(())
{
    callback_func_t handler;
    void (*another_handler)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_pointer;
};

/* Nested struct for complex type graph */
struct outer_container GTY(())
{
    struct my_struct nested_struct;
    union my_union nested_union;
    
    /* TYPE_POINTER: Pointer to another GTY type */
    struct array_container *array_ptr;
    
    /* Pointer chain for recursion */
    struct outer_container *next GTY(());
    
    /* Array of pointers */
    struct my_struct *struct_array[5] GTY(());
};

/* Complete the forward declaration - this creates TYPE_UNDEFINED initially */
struct forward_declared_struct GTY(())
{
    struct my_struct *back_ptr;
    struct outer_container *container_ptr;
};

/* Another complex nested type */
struct recursive_container GTY(())
{
    struct recursive_container *self_ptr GTY(());
    struct forward_declared_struct *fwd_ptr;
    
    /* Nested anonymous union */
    union {
        int anon_int;
        struct my_struct *anon_struct_ptr;
    } GTY(()) anon_union;
    
    /* Nested anonymous struct */
    struct {
        int nested_data;
        char *nested_string GTY((length("nested_len")));
        int nested_len;
    } GTY(()) anon_struct;
};

#endif /* TEST_GENGYPE_H */
