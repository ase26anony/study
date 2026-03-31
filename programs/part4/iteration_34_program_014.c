#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stddef.h>

/* TYPE_SCALAR: Basic scalar type */
typedef int my_scalar GTY(());
typedef unsigned long my_other_scalar GTY(());

/* TYPE_STRING: String type with length annotation */
struct string_struct GTY(())
{
    char* data GTY((length("str_len")));
    size_t str_len;
};

/* TYPE_STRUCT: Plain C struct */
struct plain_struct GTY(())
{
    int field1;
    double field2;
    my_scalar field3;
};

/* TYPE_USER_STRUCT: User-defined struct with custom marker */
struct user_defined_struct GTY((user))
{
    void* custom_data;
    int user_tag;
};

/* TYPE_UNION: Union type */
union my_union GTY(())
{
    int as_int;
    double as_double;
    void* as_ptr;
    struct plain_struct* as_struct;
};

/* TYPE_POINTER: Pointer types */
struct pointer_container GTY(())
{
    struct plain_struct* ptr_to_struct GTY(());
    union my_union* ptr_to_union GTY(());
    struct string_struct** double_ptr GTY(());
    void* opaque_ptr GTY((skip));
};

/* TYPE_ARRAY: Array types */
struct array_container GTY(())
{
    /* Fixed-size array */
    int fixed_array[10];
    
    /* Variable-length array with length annotation */
    int* var_array GTY((length("var_len")));
    size_t var_len;
    
    /* Array of pointers */
    struct plain_struct* struct_array[5] GTY(());
    
    /* Nested array in struct */
    struct {
        char nested_array[20];
    } nested;
};

/* TYPE_CALLBACK: Callback/function pointer types */
typedef void (*simple_callback)(int, double) GTY((callback));

struct callback_container GTY(())
{
    simple_callback cb1;
    void (*cb2)(struct plain_struct*, union my_union*) GTY((callback));
    
    /* Callback with user data */
    void (*user_cb)(void* user_data, int result) GTY((callback));
    void* user_data GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific struct */
struct lang_specific_struct GTY((lang_struct))
{
    int lang_specific_field;
    void* lang_data GTY((skip));
    
    /* Nested language-specific type */
    struct {
        int nested_lang_field;
    } GTY((lang_struct)) nested_lang;
};

/* Complex type that combines multiple kinds */
struct master_container GTY(())
{
    /* Scalar */
    my_scalar scalar_field;
    
    /* String */
    struct string_struct str_field;
    
    /* Struct */
    struct plain_struct plain_field;
    
    /* User struct */
    struct user_defined_struct* user_field GTY(());
    
    /* Union */
    union my_union union_field;
    
    /* Pointer */
    struct pointer_container* ptr_field GTY(());
    
    /* Array */
    struct array_container array_field;
    
    /* Callback */
    struct callback_container callback_field;
    
    /* Language struct */
    struct lang_specific_struct* lang_field GTY(());
    
    /* Self-referential pointer for type graph */
    struct master_container* next GTY(());
};

/* Forward declarations for complex type graphs */
struct forward_declared GTY(());
struct another_forward GTY(());

struct forward_declared
{
    int data;
    struct another_forward* link GTY(());
};

struct another_forward
{
    char* name GTY((length("name_len")));
    size_t name_len;
    struct forward_declared* backlink GTY(());
};

/* Enumeration type (should be treated as scalar) */
typedef enum {
    ENUM_VALUE1,
    ENUM_VALUE2,
    ENUM_VALUE3
} my_enum GTY(());

/* Bitfield struct */
struct bitfield_struct GTY(())
{
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* TYPE_UNDEFINED: This might be triggered by incomplete types or special cases */
/* We'll create a typedef to an undefined struct */
typedef struct undefined_struct undefined_type;

/* Function prototypes that use GTY types */
void process_struct(struct plain_struct* ps GTY(()));
void handle_callback(simple_callback cb);
struct master_container* create_container(void) GTY((returns_nonnull));

#endif /* TEST_TYPES_H */
