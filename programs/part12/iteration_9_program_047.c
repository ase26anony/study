/* test-gtype-coverage.c - Comprehensive type coverage for gengtype-state.cc
 * This file should be placed in the gcc/ directory and processed during GCC build.
 * It contains GTY-annotated types covering all TYPE_* enum values.
 */

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
char GTY(()) string_array[] = "array string";

/* TYPE_POINTER: Various pointer types */
typedef scalar_int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef struct opaque_struct* GTY(()) opaque_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*compare_func)(const void*, const void*);
typedef void GTY((callback)) (*void_callback)(void);

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef int GTY(()) array_of_ptrs[5];

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int i;
    float f;
    void* p;
};

union GTY(()) tagged_union {
    enum { TAG_INT, TAG_PTR } tag;
    struct {
        int value;
    } GTY((desc("%0.tag"))) as_int;
    struct {
        void* ptr;
    } GTY((desc("%0.tag"))) as_ptr;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    int id;
    char* GTY((length("strlen(%h.name)+1"))) name;
    scalar_int value;
};

struct GTY((chain_next("%h.next"))) linked_node {
    int data;
    struct linked_node* GTY((skip)) next;
};

struct GTY(()) complex_struct {
    /* Nested types to create dependencies */
    union basic_union u;
    struct simple_struct s;
    int_ptr ptr_array[5];
    compare_func comparator;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
struct GTY((user)) user_defined_struct {
    int magic;
    void* GTY((user)) user_data;
    struct complex_struct* cs;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structs */
/* Using GCC vector extension to trigger lang_struct handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) tree_common {
    int code;
    union tree_node* GTY((skip)) chain;
};

/* Recursive and nested type definitions for complex type graphs */
struct GTY(()) container {
    /* Array of pointers to structs */
    struct simple_struct* GTY((length("%h.count"))) items[10];
    int count;
    
    /* Union field */
    union tagged_union tag_union;
    
    /* Callback function pointer */
    compare_func sorter;
    
    /* Pointer to array */
    int (*GTY(()) matrix_ptr)[5][5];
    
    /* Nested struct */
    struct {
        int depth;
        struct container* GTY((skip)) parent;
    } GTY(()) nesting;
};

/* More complex type relationships */
typedef struct GTY(()) recursive_struct {
    int value;
    struct recursive_struct* GTY((skip)) left;
    struct recursive_struct* GTY((skip)) right;
    void_callback on_event;
} tree_node;

/* Array of unions */
union GTY(()) variant_array[5];

/* Struct containing all type kinds */
struct GTY(()) type_kitchen_sink {
    /* SCALAR */
    color_enum color;
    
    /* STRING */
    char* GTY((length("strlen(%h.text)+1"))) text;
    
    /* POINTER */
    void_ptr generic_ptr;
    
    /* ARRAY */
    int GTY(()) matrix[3][3];
    
    /* UNION */
    union basic_union data;
    
    /* STRUCT */
    struct simple_struct simple;
    
    /* CALLBACK */
    compare_func compare;
    
    /* Nested USER_STRUCT */
    struct user_defined_struct* GTY((skip)) user;
    
    /* LANG_STRUCT-like */
    v4si vector;
};

/* Function pointer returning struct */
struct simple_struct* GTY((callback)) (*struct_factory)(int id);

/* Typedef chain leading to various types */
typedef int GTY(()) base_type;
typedef base_type* GTY(()) ptr_to_base;
typedef ptr_to_base GTY(()) array_of_ptrs[10];
typedef array_of_ptrs* GTY(()) ptr_to_array_of_ptrs;

/* Incomplete array in struct */
struct GTY(()) flex_array_struct {
    int length;
    int GTY((length("%h.length"))) data[];
};

/* Multiple indirection */
typedef struct container**** GTY(()) deep_ptr_chain;

/* Enum in struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    color_enum color : 3;
};

/* Union with struct members */
union GTY(()) struct_union {
    struct simple_struct s;
    struct complex_struct c;
    struct container cnt;
};

/* Array of function pointers */
compare_func GTY(()) callbacks[5];

/* Struct with union containing array */
struct GTY(()) union_with_array {
    enum { INT_ARRAY, PTR_ARRAY } type;
    union {
        int GTY(()) ints[10];
        void* GTY(()) ptrs[10];
    } GTY((desc("%0.type"))) data;
};

/* Forward declared struct that's later defined */
struct GTY(()) forward_declared;
struct GTY(()) uses_forward {
    struct forward_declared* GTY((skip)) fwd_ptr;
};

struct GTY(()) forward_declared {
    int defined;
    struct uses_forward* GTY((skip)) back_ref;
};

/* Language-specific type using attribute */
struct GTY(()) gcc_internal_type {
    int GTY((tag("0"))) code;
    union {
        int ival;
        float fval;
        char* GTY((length("strlen(%0.sval)+1"))) sval;
    } GTY((desc("%0.code"))) u;
} __attribute__((aligned(16)));

/* Template for generating multiple instances */
#define DECLARE_STRUCT_TYPE(name, field_type) \
    struct GTY(()) name##_struct { \
        int id; \
        field_type GTY(()) data; \
    }

DECLARE_STRUCT_TYPE(int_wrapper, int);
DECLARE_STRUCT_TYPE(ptr_wrapper, void*);
DECLARE_STRUCT_TYPE(array_wrapper, int[5]);

/* Extern declarations to create undefined references */
extern struct GTY(()) external_struct;
extern union GTY(()) external_union;

/* Global variables with various types */
struct type_kitchen_sink GTY(()) global_sink;
tree_node* GTY(()) global_tree = NULL;
compare_func GTY(()) global_comparator = NULL;
