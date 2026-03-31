#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int GTY(()) my_scalar;

/* TYPE_STRING: String type with length attribute */
struct GTY(()) string_struct {
    char* GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Regular struct */
struct GTY(()) regular_struct {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with callback */
struct GTY((user)) user_struct {
    void* data;
    int (*compare)(const void*, const void*);
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int i;
    double d;
    char* GTY((tag("0"))) str;
    struct regular_struct* GTY((tag("1"))) rs;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct regular_struct* GTY(()) ptr1;
    struct string_struct** GTY(()) ptr2;
    void (*callback)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int GTY((length("array_len"))) *dynamic_array;
    size_t array_len;
    int fixed_array[10];
    struct regular_struct struct_array[5];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*GTY((callback)) callback_func)(int, void*);

struct GTY(()) callback_container {
    callback_func func;
    void* user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
extern "C" {
#endif

struct GTY((lang_struct)) lang_specific {
    int language_specific_field;
    void* language_data;
};

#ifdef __cplusplus
}
#endif

/* Complex nested type to ensure all categories are traversed */
struct GTY(()) master_container {
    my_scalar scalar_field;
    struct string_struct string_field;
    struct regular_struct struct_field;
    struct user_struct* user_struct_ptr;
    union my_union union_field;
    struct pointer_struct* pointer_field;
    struct array_struct array_field;
    struct callback_container callback_field;
    struct lang_specific* lang_field;
    
    /* Undefined reference - will create TYPE_UNDEFINED */
    struct undefined_type* GTY(()) undefined_ptr;
};

/* Function pointer typedef */
typedef int (*comparator_func)(const void*, const void*);

/* Another callback example */
void GTY((callback)) example_callback(int value, void* context);

#endif /* TEST_TYPES_H */
