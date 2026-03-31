#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Forward declarations to create TYPE_UNDEFINED cases */
struct forward_declared_struct GTY(());
typedef struct forward_declared_struct *forward_ptr_t GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(())
{
    my_scalar_t field1;
    another_scalar_t field2;
};

/* Nested struct for complexity */
struct outer_struct GTY(())
{
    struct base_struct base GTY(());
    
    /* TYPE_ARRAY: Array within struct */
    struct base_struct array_field[10] GTY(());
    
    /* Zero-length array */
    int flexible_array[] GTY(());
};

/* TYPE_USER_STRUCT: Marked for special user handling */
struct user_handled_struct GTY((user))
{
    int user_data;
    char user_name[32];
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(())
{
    my_scalar_t as_scalar;
    struct base_struct *as_ptr GTY(());
    char as_string[64];
};

/* TYPE_POINTER: Struct containing pointers, creating circular references */
struct pointer_struct GTY(())
{
    /* Pointer to self for recursion */
    struct pointer_struct *next GTY(());
    
    /* Pointer to forward-declared type */
    forward_ptr_t forward_ref GTY(());
    
    /* Pointer to union */
    union data_union *union_ptr GTY(());
};

/* Now define the forward-declared struct to resolve TYPE_UNDEFINED */
struct forward_declared_struct GTY(())
{
    int resolved_field;
    struct pointer_struct *back_ref GTY(());
};

/* TYPE_ARRAY: Special array with length attribute */
struct array_container GTY(())
{
    /* Array with GTY length attribute */
    int *dynamic_array GTY((length("array_length")));
    int array_length;
    
    /* Fixed-size array */
    char fixed_array[100] GTY(());
};

/* TYPE_STRING: String type with length attribute */
struct string_container GTY(())
{
    /* String field with length attribute */
    char *string_field GTY((length("str_len")));
    int str_len;
    
    const char *const_string GTY(());
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, char *) GTY((callback));

struct callback_container GTY(())
{
    callback_func_t handler GTY(());
    
    /* Alternative: direct function pointer field */
    int (*direct_callback)(struct base_struct *) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void *lang_private GTY(());
};

/* Complex nested type for maximum coverage */
struct master_container GTY(())
{
    /* Include all major types */
    struct base_struct base GTY(());
    struct user_handled_struct user GTY(());
    union data_union data GTY(());
    struct pointer_struct *ptr_field GTY(());
    struct array_container arrays GTY(());
    struct string_container strings GTY(());
    struct callback_container callbacks GTY(());
    struct lang_specific_struct lang GTY(());
    
    /* Nested anonymous union */
    union GTY(())
    {
        int anon_int;
        struct base_struct *anon_ptr GTY(());
    } anonymous_union;
    
    /* Nested anonymous struct */
    struct GTY(())
    {
        int nested_int;
        char nested_char;
    } anonymous_struct;
};

#endif /* TEST_GENGYPE_H */
