#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type definitions */
typedef int my_scalar;
typedef long my_long_scalar;
typedef unsigned char my_uchar_scalar;

/* TYPE_POINTER: Pointer type definition */
typedef my_scalar *my_scalar_ptr;

/* TYPE_ARRAY: Array type definition */
typedef int my_int_array[10];

/* TYPE_CALLBACK: Function pointer/callback type */
typedef void (*my_callback)(int, const char*);

/* TYPE_STRING: String type with GTY annotation */
struct GTY(()) string_struct {
    const char * GTY((length("str_len + 1"))) str_data;
    size_t str_len;
};

/* TYPE_STRUCT: Regular struct with GTY annotation */
struct GTY(()) my_struct {
    my_scalar field1;
    my_scalar_ptr field2;
    my_int_array field3;
    struct string_struct field4;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) my_user_struct {
    int user_id;
    void *user_data;  /* User handles memory management */
};

/* TYPE_UNION: Union type with GTY annotation */
union GTY(()) my_union {
    my_scalar as_scalar;
    my_scalar_ptr as_pointer;
    struct my_struct as_struct;
};

/* TYPE_LANG_STRUCT: Language-specific struct (GCC extension) */
#ifdef __GNUC__
struct GTY((lang_struct)) my_lang_struct {
    int lang_specific_field;
    __extension__ struct {
        int extended_field;
    } ext;
};
#endif

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_container {
    /* TYPE_ARRAY of structs */
    struct my_struct GTY((length("array_count"))) *struct_array;
    size_t array_count;
    
    /* TYPE_POINTER to union */
    union my_union *union_ptr;
    
    /* TYPE_CALLBACK field */
    my_callback callback_func;
    
    /* Nested struct with string */
    struct {
        char * GTY((length("name_len"))) name;
        size_t name_len;
    } nested;
};

/* Forward declaration for pointer chain */
struct GTY(()) forward_declared;
struct GTY(()) forward_declared {
    int value;
    struct forward_declared *next;  /* TYPE_POINTER to same type */
};

/* Variable length array struct */
struct GTY(()) var_len_array_struct {
    int count;
    int GTY((length("count"))) items[];
};

/* Another callback type for variety */
typedef int (*comparator_func)(const void*, const void*);

/* Struct with multiple callback pointers */
struct GTY(()) callback_container {
    my_callback cb1;
    comparator_func cb2;
    void (*cb3)(void);
};

/* Union containing different pointer types */
union GTY(()) pointer_union {
    struct my_struct *struct_ptr;
    struct string_struct *string_ptr;
    void *generic_ptr;
    my_callback callback_ptr;
};

#endif /* TEST_TYPES_H */
