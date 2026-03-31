#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "config.h"
#include "system.h"

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct opaque;
typedef struct opaque *opaque_ptr_t GTY(());

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t GTY(());
typedef long scalar_long_t GTY(());
typedef double scalar_double_t GTY(());
typedef float scalar_float_t GTY(());
typedef char scalar_char_t GTY(());
typedef unsigned int scalar_uint_t GTY(());

/* Enum type (also scalar) */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t GTY(());

/* TYPE_STRING: String types */
typedef const char *const_string_t GTY(());
typedef char *string_t GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_t)(int, void*) GTY(());
typedef int (*compare_fn_t)(const void*, const void*) GTY(());

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *struct_ptr_t GTY(());
typedef void *generic_ptr_t GTY(());
typedef int *int_ptr_t GTY(());

/* TYPE_ARRAY: Array types */
typedef int int_array_t[10] GTY(());
typedef struct basic_struct *struct_ptr_array_t[5] GTY(());

/* TYPE_UNION: Union types */
typedef union data_union {
    int i;
    float f;
    void *p;
} data_union_t GTY(());

/* Union with discriminator for TYPE_UNION with GTY((desc)) */
struct tagged_union_container GTY(());
typedef struct tagged_union_container {
    int tag;
    union {
        int int_value GTY((tag("0")));
        float float_value GTY((tag("1")));
        char *string_value GTY((tag("2")));
    } data GTY((desc("tag")));
} tagged_union_container_t;

/* TYPE_STRUCT: Basic structure */
struct basic_struct GTY(());
typedef struct basic_struct {
    int id;
    char *name GTY(());
    struct basic_struct *next GTY(());
    int_array_t scores;
    callback_t handler GTY(());
} basic_struct_t;

/* TYPE_USER_STRUCT: User-defined structure with custom traversal */
struct user_struct GTY((user));
typedef struct user_struct {
    void *custom_data;
    int (*user_traverse)(void*) GTY(());
} user_struct_t;

/* TYPE_LANG_STRUCT: Language-specific structure (C++ style) */
#ifdef __cplusplus
class lang_struct_base GTY(());
class lang_struct_base {
public:
    virtual ~lang_struct_base() {}
    int base_value GTY(());
    
    virtual void method() = 0;
};

class lang_struct_derived GTY(());
class lang_struct_derived : public lang_struct_base {
public:
    char *derived_name GTY(());
    
    void method() override;
};
#else
/* For C, simulate language-specific with special GTY markup */
struct lang_struct GTY(());
typedef struct lang_struct {
    int lang_specific_field GTY(());
    void *vtable GTY((skip));  /* Simulate C++ vtable */
} lang_struct_t;
#endif

/* Complex nested structure covering multiple types */
struct container GTY(());
typedef struct container {
    /* TYPE_STRUCT */
    basic_struct_t item GTY(());
    
    /* TYPE_POINTER */
    struct container *next GTY(());
    struct container *prev GTY(());
    
    /* TYPE_ARRAY */
    struct_ptr_array_t pointers;
    
    /* TYPE_UNION */
    data_union_t union_data GTY(());
    
    /* TYPE_CALLBACK */
    compare_fn_t compare GTY(());
    
    /* TYPE_STRING */
    const_string_t description GTY(());
    
    /* TYPE_SCALAR */
    scalar_int_t count;
    color_t color;
    
    /* TYPE_UNDEFINED reference */
    opaque_ptr_t opaque_ref GTY(());
} container_t;

/* Linked list structure for recursive traversal */
struct list_node GTY(());
typedef struct list_node {
    int value;
    struct list_node *next GTY(());
    struct list_node *prev GTY(());
} list_node_t;

/* Tree structure */
struct tree_node GTY(());
typedef struct tree_node {
    int key;
    char *data GTY(());
    struct tree_node *left GTY(());
    struct tree_node *right GTY(());
    struct tree_node *parent GTY(());
} tree_node_t;

/* Variable length array structure */
struct var_array GTY(());
typedef struct var_array {
    int length;
    int data[1] GTY((length("%0.length")));
} var_array_t;

#endif /* TEST_TYPES_H */
