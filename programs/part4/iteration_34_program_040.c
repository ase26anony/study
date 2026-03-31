#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());

/* TYPE_UNDEFINED: Forward declaration without definition */
struct undefined_struct GTY(());

/* TYPE_STRUCT: Plain C struct */
struct my_struct GTY(())
{
    int field1;
    char field2;
    my_scalar scalar_field;
};

/* TYPE_USER_STRUCT: Struct with user marker */
struct user_marked_struct GTY((user))
{
    int user_data;
    void* user_ptr;
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
    int int_val;
    char char_val;
    double double_val;
    struct my_struct* struct_ptr;
};

/* TYPE_POINTER: Pointer types */
struct pointer_struct GTY(())
{
    struct my_struct* direct_ptr GTY((skip));
    struct my_struct** double_ptr;
    union my_union* union_ptr;
};

/* TYPE_ARRAY: Array types */
struct array_struct GTY(())
{
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length callback */
    char* vla_string GTY((length("strlen(%h.vla_string) + 1")));
    
    /* Pointer to array */
    int (*array_ptr)[5];
    
    /* Array of pointers */
    struct my_struct* struct_array[8];
};

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(())
{
    char* regular_string;
    const char* const_string GTY((length("strlen(%h.const_string)")));
    char string_array[3][20];
};

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*callback_func)(int, char*) GTY((callback));

struct callback_struct GTY(())
{
    callback_func handler;
    void (*direct_callback)(struct my_struct*) GTY((callback));
    int (*compare_func)(const void*, const void*);
};

/* TYPE_LANG_STRUCT: Language-specific struct */
#ifdef __cplusplus
extern "C" {
#endif

struct lang_specific_struct GTY((lang_struct))
{
    int lang_data;
    void* lang_pointer;
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int nested_field;
        char nested_char;
    } anonymous;
};

/* Complex nested type to ensure deep traversal */
struct container_struct GTY(())
{
    /* Self-referential pointer */
    struct container_struct* next;
    
    /* Union containing different types */
    union {
        struct my_struct as_struct;
        struct array_struct as_array;
        struct string_struct as_string;
    } data;
    
    /* Array of unions */
    union my_union union_array[4];
    
    /* Pointer to callback */
    callback_func* callback_ptr;
    
    /* Multi-dimensional array */
    int matrix[3][3];
};

/* Template-like macro for generating parameterized types */
#define DECLARE_PAIR_TYPE(T1, T2) \
    struct pair_##T1##_##T2 GTY(()) { \
        T1 first; \
        T2 second; \
    }

/* Generate some pair types */
DECLARE_PAIR_TYPE(int, char*);
DECLARE_PAIR_TYPE(struct my_struct*, union my_union);
DECLARE_PAIR_TYPE(callback_func, int[5]);

/* Function pointer typedefs */
typedef int (*binary_op)(int, int) GTY(());

/* Enum type (should be treated as scalar) */
typedef enum {
    STATE_A,
    STATE_B,
    STATE_C
} my_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(())
{
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Aligned struct with GCC attributes */
struct aligned_struct GTY(())
{
    int data __attribute__((aligned(16)));
    char buffer[32] __attribute__((aligned(8)));
} __attribute__((packed));

/* Volatile and const qualified types */
struct qualified_struct GTY(())
{
    volatile int volatile_member;
    const char* const_ptr_member;
    const volatile int cv_member;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_TYPES_H */
