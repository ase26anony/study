#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_UNDEFINED: Forward declaration that won't be defined */
struct undefined_struct GTY(());

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar_t GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(())
{
    my_scalar_t field1;
    another_scalar_t field2;
};

/* TYPE_USER_STRUCT: Struct with user attribute */
struct user_handled_struct GTY((user))
{
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union with GTY-tagged members */
union data_union GTY(())
{
    my_scalar_t as_scalar;
    struct base_struct* as_struct_ptr;
    char* as_string;
};

/* TYPE_POINTER: Struct containing pointers (including recursive/circular) */
struct pointer_struct GTY(())
{
    struct base_struct* direct_ptr;
    struct pointer_struct* self_ptr;  /* Recursive pointer */
    struct undefined_struct* to_undefined;  /* Pointer to undefined type */
    union data_union* to_union;
};

/* TYPE_ARRAY: Struct containing various array types */
struct array_struct GTY(())
{
    int fixed_array[10];
    struct base_struct* ptr_array[5];
    int zero_length_array[0];
    /* Variable length array with length attribute */
    int* variable_array GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRING: Struct with string field */
struct string_struct GTY(())
{
    char* name GTY((length("name_len")));
    int name_len;
    const char* const_string;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func_t)(int, void*) GTY((callback));

struct callback_struct GTY(())
{
    callback_func_t handler;
    void (*regular_func_ptr)(void);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct (1)))
{
    int lang_data;
    void* lang_ptr;
};

/* Nested types for complexity */
struct container_struct GTY(())
{
    /* Nested struct */
    struct {
        int nested_field;
        struct pointer_struct* nested_ptr;
    } inner GTY(());
    
    /* Nested union */
    union {
        int as_int;
        float as_float;
    } data_union GTY(());
    
    /* Array of structs */
    struct base_struct struct_array[3];
    
    /* Pointer to callback */
    callback_func_t callback_array[2];
};

/* Another forward declaration for more TYPE_UNDEFINED coverage */
union forward_declared_union GTY(());

/* Struct that references forward-declared union */
struct references_forward GTY(())
{
    union forward_declared_union* fwd_ptr;
    struct undefined_struct* another_undefined;
};

#endif /* TEST_GENGYPE_H */
