/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct GTY(()) opaque_struct;
union GTY(()) opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int GTY(()) scalar_int;
typedef char GTY(()) scalar_char;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
const char GTY(()) *string_ptr = "test string";
char GTY(()) string_array[] = "hello world";

/* TYPE_POINTER: Various pointer types */
typedef scalar_int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef struct opaque_struct* GTY(()) opaque_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_fn)(const void*, const void*);
typedef void GTY((callback)) (*void_callback)(void);

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef int GTY(()) matrix[5][5];

/* TYPE_UNION: Union types */
union GTY(()) my_union {
    int i;
    float f;
    void* p;
    scalar_int* ip;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) my_struct {
    /* Scalar field */
    scalar_int id;
    
    /* String field */
    const char* GTY(()) name;
    
    /* Pointer field */
    struct my_struct* GTY(()) next;
    
    /* Array field */
    int GTY(()) scores[5];
    
    /* Union field */
    union my_union GTY(()) data;
    
    /* Callback field */
    compare_fn GTY(()) comparator;
};

/* Recursive struct for chain_next testing */
struct GTY((chain_next("%h.next"))) linked_node {
    int value;
    struct linked_node* GTY(()) next;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
/* Using GCC vector extension to trigger special type handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) user_defined {
    v4si vector_data;
    int GTY(()) user_tag;
};

/* TYPE_LANG_STRUCT: GCC internal language structures */
/* Mimic tree-like structure used in GCC internals */
struct GTY(()) tree_common {
    int code;
    union tree_node* GTY(()) chain;
    union tree_node* GTY(())) type;
};

struct GTY(()) tree_int_cst {
    struct tree_common common;
    HOST_WIDE_INT int_cst;
};

union GTY((desc("TREE_CODE(%h.code)"))) tree_node {
    struct tree_common GTY((skip)) common;
    struct tree_int_cst int_cst;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) container {
    /* Array of pointers to structs */
    struct my_struct* GTY(()) items[10];
    
    /* Pointer to array */
    int (*GTY(()) matrix_ptr)[5];
    
    /* Union containing struct */
    union {
        struct my_struct GTY(()) s;
        union my_union GTY(()) u;
    } GTY(())) variant;
    
    /* Callback returning pointer */
    struct my_struct* (*GTY((callback)) allocator)(int);
    
    /* Nested struct */
    struct {
        int depth;
        struct container* GTY(()) parent;
    } GTY(())) nested;
};

/* Function pointer with complex return type */
union my_union* (*GTY((callback)) complex_callback)(
    struct container*,
    compare_fn
);

/* Template for generating multiple type instances */
#define DECLARE_STRUCT_TYPE(n) \
    struct GTY(()) struct_##n { \
        int id_##n; \
        struct struct_##n* GTY(()) next_##n; \
    }

DECLARE_STRUCT_TYPE(1);
DECLARE_STRUCT_TYPE(2);
DECLARE_STRUCT_TYPE(3);

/* Global variables with GTY markers */
struct my_struct GTY(()) global_struct = {0};
union my_union GTY(()) global_union;
struct container* GTY(()) global_container;

/* Array of unions */
union my_union GTY(()) union_array[5];

/* Pointer to function returning struct */
struct my_struct* (*GTY(()) struct_maker)(void);

/* Self-referential type for cycle testing */
struct GTY(()) cyclic {
    int value;
    struct cyclic* GTY(()) self_ref;
    struct cyclic* GTY(()) next_cyclic;
};

/* Type with conditional fields */
struct GTY(()) conditional_struct {
    int type;
    union {
        int GTY(()) int_val;
        const char* GTY(()) str_val;
        void* GTY(()) ptr_val;
    } GTY((desc("%0.type"))) data;
};

/* Mark all types for garbage collection */
void GTY(()) register_types(void) {
    /* This function doesn't need implementation.
       Its purpose is to force gengtype to process
       all the types referenced in its declaration. */
    struct my_struct s;
    union my_union u;
    struct container c;
    struct cyclic cyc;
    struct conditional_struct cs;
    
    /* Reference all types to ensure they're processed */
    (void)s;
    (void)u;
    (void)c;
    (void)cyc;
    (void)cs;
}
