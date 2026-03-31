/* test-gty.h - Comprehensive GTY type definitions for coverage testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include all specialized type definition headers */
#include "scalar-types.h"
#include "string-types.h"
#include "struct-types.h"
#include "user-struct-types.h"
#include "union-types.h"
#include "pointer-types.h"
#include "array-types.h"
#include "callback-types.h"
#include "lang-struct-types.h"
#include "complex-nested-types.h"
#include "macro-generated-types.h"

/* Forward declarations for complex type relationships */
struct GTY(()) base_struct;
union GTY(()) base_union;
typedef void (*GTY(()) base_callback)(int);

/* Edge case: typedef that could be ambiguous */
typedef GTY(()) const char * const_string_ptr_t;

/* Another edge case: pointer to const pointer */
typedef GTY(()) const int * const * double_const_ptr_t;

/* Type with skip annotation */
struct GTY(()) skip_example {
    int data;
    void *GTY((skip)) opaque;
    struct skip_example *GTY((skip)) next_skip;
};

/* Type with maybe_undef annotation */
struct GTY(()) maybe_undef_example {
    int value;
    struct base_struct *GTY((maybe_undef)) optional_ptr;
};

/* Chain_next and chain_prev for linked lists */
struct GTY((chain_next("%h.next"))) chain_example {
    int id;
    struct chain_example *next;
    struct chain_example *GTY((skip)) prev;
};

/* Nested anonymous struct/union */
struct GTY(()) nested_anonymous {
    int tag;
    union {
        int i;
        float f;
        void *GTY((skip)) p;
    } data;
    struct {
        int x;
        int y;
    } point;
};

/* Variable length array at end of struct */
struct GTY(()) vla_struct {
    int count;
    int data[1];  /* Actually variable length */
};

/* Self-referential types */
struct GTY(()) self_ref {
    int value;
    struct self_ref *next;
    struct self_ref *GTY((skip)) prev;
};

/* Mutual recursion */
struct GTY(()) type_a;
struct GTY(()) type_b;

struct GTY(()) type_a {
    int id;
    struct type_b *partner;
};

struct GTY(()) type_b {
    int id;
    struct type_a *partner;
};

/* Template-like macro for generating families of types */
#define DEFINE_CONTAINER(TYPE, NAME) \
    struct GTY(()) NAME { \
        TYPE element; \
        struct NAME *next; \
    };

DEFINE_CONTAINER(int, int_container)
DEFINE_CONTAINER(struct type_a, a_container)
DEFINE_CONTAINER(void *, ptr_container)

/* Type with array of pointers */
struct GTY(()) array_of_ptrs {
    int count;
    void *GTY((length("%0.count"))) pointers[10];
};

/* Type with nested array */
struct GTY(()) nested_array_example {
    int matrix[3][3];
    struct type_a *objects[5];
};

/* Use all defined types in a comprehensive structure */
struct GTY(()) comprehensive_example {
    /* Scalar types */
    int_scalar_t scalar1;
    unsigned_scalar_t scalar2;
    
    /* String types */
    my_string_t string1;
    const_string_ptr_t string2;
    
    /* Structure types */
    struct my_struct struct1;
    struct complex_struct struct2;
    
    /* Pointer types */
    struct_ptr_t ptr1;
    double_ptr_t ptr2;
    
    /* Array types */
    my_array_t array1;
    struct_array_t array2;
    
    /* Union types */
    union my_union union1;
    union container union2;
    
    /* Callback types */
    callback_fn callback1;
    complex_callback_fn callback2;
    
    /* User structure pointer */
    user_struct_ptr_t user_ptr;
    
    /* Self-reference */
    struct comprehensive_example *next;
    
    /* Language structure-like */
    struct chain_example *chain_head;
};

#endif /* TEST_GTY_H */
