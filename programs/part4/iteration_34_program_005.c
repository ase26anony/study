#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());
typedef unsigned long another_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char* GTY((length("str_len"))) data;
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) simple_struct {
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_defined_struct {
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int int_val;
    double double_val;
    char* GTY((tag("0"))) string_val;
    struct simple_struct* GTY((tag("1"))) struct_ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct simple_struct* GTY(()) ptr_to_struct;
    my_scalar* GTY(()) ptr_to_scalar;
    struct string_struct** GTY(()) ptr_to_ptr;
    void (*GTY((callback("my_callback"))) callback_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int GTY(()) fixed_array[10];
    char* GTY((length("dynamic_len"))) dynamic_array;
    size_t dynamic_len;
    struct simple_struct GTY(()) struct_array[5];
};

/* TYPE_CALLBACK: Callback/function pointer type */
typedef void (*GTY((callback)) callback_func)(int, const char*);

struct GTY(()) callback_container {
    callback_func handler;
    void (*GTY((callback)) another_handler)(struct simple_struct*);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct("C"))) lang_specific_struct {
    int lang_specific_field;
    void* lang_specific_data;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct GTY(()) forward_declared_struct;

/* Complex nested type to ensure deep traversal */
struct GTY(()) complex_nested {
    union my_union GTY(()) u;
    struct array_struct GTY(()) arr;
    struct pointer_struct* GTY(()) ptrs;
    struct GTY(()) inner {
        int x;
        struct complex_nested* GTY(()) next;
    } inner_struct;
};

/* Complete the forward declaration */
struct GTY(()) forward_declared_struct {
    int complete_field;
    struct complex_nested* GTY(()) nested;
};

/* Variable-length struct with GTY annotations */
struct GTY(()) var_len_struct {
    int count;
    int GTY((length("count"))) items[];
};

/* Another callback type definition */
typedef int (*GTY((callback)) compare_func)(const void*, const void*);

/* Template-like macro to generate multiple struct types */
#define DECLARE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name##_struct { \
        field_type GTY(()) data; \
        struct name##_struct* GTY(()) next; \
    }

DECLARE_STRUCT_TYPE(int_list, int);
DECLARE_STRUCT_TYPE(string_list, char*);

/* Enumeration type (also scalar) */
typedef enum GTY(()) my_enum {
    ENUM_VAL1,
    ENUM_VAL2,
    ENUM_VAL3
} my_enum_type;

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
    unsigned int padding:26;
};

#endif /* TEST_TYPES_H */
