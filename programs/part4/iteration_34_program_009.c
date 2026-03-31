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

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) plain_struct {
    int x;
    double y;
    my_scalar z;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct GTY((user)) user_struct {
    void* custom_data;
    int (*custom_func)(void);
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int as_int;
    double as_double;
    void* GTY((tag("0"))) as_ptr;
};

/* TYPE_POINTER: Pointer types */
struct GTY(()) pointer_struct {
    struct plain_struct* GTY((skip)) ptr1;
    struct string_struct** GTY((skip)) ptr2;
    void (*GTY((skip)) callback_ptr)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) array_struct {
    int fixed_array[10];
    int* GTY((length("dynamic_len"))) dynamic_array;
    size_t dynamic_len;
    
    /* Variable length array in struct */
    struct GTY(()) {
        int count;
        int items GTY((variable_length));
    } vla;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*GTY((callback)) callback_func)(int, const char*);

/* TYPE_LANG_STRUCT: Language-specific struct */
struct GTY((lang_struct)) lang_specific {
    int lang_specific_field;
    void* lang_data;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
struct GTY(()) forward_declared;

/* Complex type with nested structures */
struct GTY(()) container {
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Plain struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_struct* user_field;
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    struct pointer_struct* pointer_field;
    
    /* Array */
    struct array_struct array_field;
    
    /* Callback */
    callback_func callback_field;
    
    /* Language struct */
    struct lang_specific* lang_field;
    
    /* Forward declared (undefined until defined later) */
    struct forward_declared* forward_field;
    
    /* Self-referential pointer */
    struct container* GTY((skip)) self_ptr;
    
    /* Array of pointers */
    struct plain_struct* GTY((length("ptr_count"))) ptr_array[5];
    int ptr_count;
};

/* Now define the forward-declared struct */
struct GTY(()) forward_declared {
    int defined_now;
    struct container* back_ref;
};

/* Another callback type using different syntax */
typedef int (GTY((callback)) *another_callback)(struct container*, int);

/* Union with GTY on individual fields */
union GTY(()) tagged_union {
    int GTY((tag("TYPE_INT"))) as_int;
    double GTY((tag("TYPE_DOUBLE"))) as_double;
    char* GTY((tag("TYPE_STRING"), length("strlen(str)"))) as_string;
};

/* Struct with nested anonymous struct */
struct GTY(()) outer_struct {
    struct GTY(()) {
        int inner_a;
        double inner_b;
    } inner;
    
    union GTY(()) {
        short s;
        long l;
    } inner_union;
};

/* Array of unions */
union GTY(()) union_array[10];

/* Function pointer table */
struct GTY(()) callback_table {
    callback_func funcs[5];
    another_callback another_func;
};

#endif /* TEST_TYPES_H */
