/* test-coverage-types.h - Comprehensive type declarations for gengtype coverage */
/* This file should be placed in the gcc/ directory and included in the build */

#ifndef TEST_COVERAGE_TYPES_H
#define TEST_COVERAGE_TYPES_H

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_struct;
union opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef _Bool scalar_bool;

/* Scalar enum type */
typedef enum color_enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} scalar_enum;

/* TYPE_STRING: String types */
const char *global_string = "test string literal";
char string_array[] = "string array initializer";

/* TYPE_STRUCT: Basic struct types */
struct GTY(()) basic_struct {
    int field1;
    char *field2;
    double field3;
};

/* Recursive struct with pointer to itself */
struct GTY(()) recursive_struct {
    int value;
    struct recursive_struct *GTY((skip)) next;
};

/* Struct with nested array */
struct GTY(()) struct_with_array {
    int count;
    int values[10];
    struct basic_struct nested;
};

/* TYPE_UNION: Basic union type */
union GTY(()) basic_union {
    int int_val;
    float float_val;
    void *ptr_val;
    char *string_val;
};

/* Tagged union with struct field */
union GTY(()) tagged_union {
    struct {
        int type;
        void *data;
    } GTY((tag("0"))) tagged;
    int raw_value;
};

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef const char *const_string_ptr;
typedef struct basic_struct *struct_ptr;
typedef union basic_union *union_ptr;

/* Function pointer typedef */
typedef int (*comparison_fn)(const void *, const void *);

/* TYPE_ARRAY: Array declarations */
extern int external_array[];
int fixed_size_array[20];
struct basic_struct struct_array[5];
int *pointer_array[10];

/* Incomplete array in struct */
struct GTY(()) struct_with_incomplete_array {
    int length;
    int data[];
};

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef void GTY((callback)) (*simple_callback)(void);
typedef int GTY((callback)) (*complex_callback)(int, const char *, void *);

/* Callback struct field */
struct GTY(()) callback_container {
    simple_callback cb;
    void *user_data;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* These typically require GCC-specific type definitions */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

/* RTL-like structure */
struct GTY(()) rtx_def {
    int code;
    int mode;
    struct rtx_def *fld[1];
};

/* Union of tree nodes (common in GCC) */
union GTY((desc ("TREE_CODE (&%h)"), variable_size)) tree_node {
    struct tree_common GTY((skip)) common;
    struct tree_decl GTY((tag ("0"))) decl;
    struct tree_type GTY((tag ("1"))) type;
};

/* Forward declaration for tree types */
struct tree_decl;
struct tree_type;

/* Complex nested type example */
struct GTY(()) complex_nested {
    /* Struct field */
    struct basic_struct nested_struct;
    
    /* Union field */
    union basic_union nested_union;
    
    /* Pointer field */
    struct complex_nested *self_ptr;
    
    /* Array field */
    int matrix[3][3];
    
    /* Function pointer field */
    complex_callback handler;
    
    /* String field */
    const char *name;
    
    /* Scalar fields */
    scalar_enum color;
    scalar_bool flag;
};

/* Container with all type kinds */
struct GTY(()) type_coverage_container {
    /* TYPE_STRUCT */
    struct complex_nested struct_field;
    
    /* TYPE_UNION */
    union tagged_union union_field;
    
    /* TYPE_POINTER */
    void_ptr void_pointer;
    comparison_fn func_pointer;
    
    /* TYPE_ARRAY */
    int int_array[5];
    struct_ptr struct_ptr_array[3];
    
    /* TYPE_SCALAR */
    scalar_int int_scalar;
    scalar_enum enum_scalar;
    scalar_bool bool_scalar;
    
    /* TYPE_STRING */
    const char *string_field;
    char string_buffer[256];
    
    /* TYPE_CALLBACK */
    simple_callback callback_field;
    
    /* TYPE_LANG_STRUCT */
    v4si vector_field;
    
    /* Reference to undefined type */
    struct opaque_struct *opaque_ref;
};

/* Function declarations using various types */
void process_struct(struct basic_struct *GTY((skip)) s);
union basic_union create_union(int type, void *data);
int compare_values(const void *a, const void *b);
void register_callback(simple_callback cb);

/* Global variables with GTY markers */
extern struct type_coverage_container GTY((length ("%h.count"))) *global_container;
extern union tree_node GTY((chain_next ("%h.next"))) *global_tree_chain;

#endif /* TEST_COVERAGE_TYPES_H */
