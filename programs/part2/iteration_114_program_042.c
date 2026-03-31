#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* TYPE_SCALAR: Fundamental types with GTY markers */
typedef unsigned long my_scalar_t GTY(());
typedef int another_scalar GTY(());

/* Forward declaration for TYPE_UNDEFINED handling */
struct forward_declared_struct GTY(());

/* TYPE_STRUCT: Standard C structs */
struct base_struct GTY(())
{
    my_scalar_t value;
    another_scalar count;
};

/* TYPE_USER_STRUCT: User-handled struct */
struct user_handled_struct GTY((user))
{
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION */
union variant_union GTY(())
{
    int int_val;
    double double_val;
    struct base_struct* struct_ptr;
};

/* TYPE_POINTER: Struct with pointer fields */
struct pointer_container GTY(())
{
    struct base_struct* direct_ptr;
    struct forward_declared_struct* forward_ptr;
    struct pointer_container* self_ptr;  /* Recursive reference */
};

/* TYPE_ARRAY: Struct with array fields */
struct array_container GTY(())
{
    int fixed_array[10];
    struct base_struct* ptr_array[5];
    int zero_length_array[0];  /* Zero-length array */
    int* variable_array GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRING: String field with length attribute */
struct string_container GTY(())
{
    char* regular_string;
    char* counted_string GTY((length("str_len")));
    int str_len;
};

/* TYPE_CALLBACK: Function pointer with callback attribute */
typedef void (*callback_func)(int, char*) GTY((callback));

struct callback_container GTY(())
{
    callback_func handler;
    void (*another_handler)(void) GTY((callback));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct(1)))
{
    int lang_data;
    void* lang_pointer;
};

/* Nested struct for complexity */
struct outer_container GTY(())
{
    struct inner_nested GTY(())
    {
        int nested_data;
        union variant_union nested_union;
    } inner;
    
    struct array_container arrays;
    struct pointer_container* pointers;
};

/* Now define the forward-declared struct (completing TYPE_UNDEFINED -> TYPE_STRUCT) */
struct forward_declared_struct GTY(())
{
    int finalized_data;
    struct outer_container* link_back;
};

/* Complex circular references */
struct circular_a GTY(())
{
    int id;
    struct circular_b* partner;
};

struct circular_b GTY(())
{
    int id;
    struct circular_a* partner;
    struct circular_b* next;
};

#endif /* TEST_GENGYPE_H */
