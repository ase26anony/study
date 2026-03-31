#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct GTY(()) string_struct {
    char* GTY((length("strlen($1)"))) data;
    int length;
};

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_struct {
    void* custom_data;
    int (*custom_func)(void*);
};

/* TYPE_UNION: Union type */
union GTY(()) test_union {
    int as_int;
    double as_double;
    void* as_ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct plain_struct* GTY((skip)) ptr1;
    struct string_struct** GTY((skip)) ptr2;
    void (*func_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int fixed_array[10];
    int* GTY((length("$1->dynamic_len"))) dynamic_array;
    size_t dynamic_len;
    
    /* Variable length array in struct */
    struct GTY(()) {
        int count;
        int items GTY((length("$1->count")))[];
    } flex;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int, void*) GTY((callback));

struct GTY(()) callback_container {
    callback_func handler;
    void* GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: Language-specific struct */
/* Using GCC's internal lang_hooks - simulate with attribute */
struct GTY((lang_struct)) lang_specific {
    int lang_id;
    void* lang_data;
};

/* Complex nested type to ensure traversal */
struct GTY(()) complex_type {
    /* Contains all kinds of types */
    my_scalar scalar_field;
    struct string_struct string_field;
    struct plain_struct struct_field;
    union test_union union_field;
    struct pointer_struct* pointer_field;
    struct array_struct array_field;
    struct callback_container callback_field;
    
    /* Self-referential pointer */
    struct complex_type* GTY((skip)) next;
    
    /* Array of pointers */
    struct plain_struct* GTY((skip)) ptr_array[5];
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
struct GTY(()) undefined_struct;
struct undefined_struct* GTY((skip)) undefined_ptr;

/* Now define it */
struct GTY(()) undefined_struct {
    int defined_now;
};

/* Enum type (also scalar) */
typedef enum {
    VALUE_A,
    VALUE_B,
    VALUE_C
} my_enum GTY(());

/* Function pointer array */
typedef int (*func_array[3])(void) GTY(());

/* Nested anonymous struct/union */
struct GTY(()) container {
    struct {
        int a;
        double b;
    } inner;
    
    union {
        long x;
        void* y;
    } data;
};

/* Bitfield struct */
struct GTY(()) bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
    unsigned int padding:26;
};

#endif /* TEST_TYPES_H */
