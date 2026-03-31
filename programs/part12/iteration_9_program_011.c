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
typedef long GTY(()) scalar_long;
typedef _Bool GTY(()) scalar_bool;
typedef enum { RED, GREEN, BLUE } GTY(()) color_enum;

/* TYPE_STRING: String types */
const char GTY(()) *string_literal = "test string";
char GTY(()) string_array[] = "array string";

/* TYPE_CALLBACK: Function pointer types */
typedef int GTY((callback)) (*callback_func)(void *);
typedef void GTY((callback)) (*void_callback)(int, const char*);

/* TYPE_POINTER: Various pointer types */
typedef int* GTY(()) int_ptr;
typedef void* GTY(()) void_ptr;
typedef struct opaque_struct* GTY(()) opaque_ptr;
typedef callback_func GTY(()) callback_ptr;

/* TYPE_ARRAY: Array types */
int GTY(()) fixed_array[10];
extern int GTY(()) incomplete_array[];
typedef int GTY(()) array_of_ints[5];
typedef struct opaque_struct* GTY(()) array_of_ptrs[8];

/* TYPE_UNION: Union types */
union GTY(()) simple_union {
    int i;
    float f;
    void* p;
};

union GTY(()) complex_union {
    int int_val;
    char char_val;
    struct opaque_struct* struct_ptr;
    callback_func callback_field;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) simple_struct {
    int a;
    char b;
    long c;
};

struct GTY((chain_next("%h.next"))) linked_struct {
    int value;
    struct linked_struct* GTY((skip)) next;
    struct opaque_struct* GTY((skip)) opaque_ref;
};

struct GTY(()) nested_struct {
    int id;
    union simple_union data;
    struct simple_struct base;
    callback_func processor;
};

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
typedef struct GTY((user)) user_defined_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        const char* string_val;
    } GTY((desc("%0.tag"))) data;
} user_struct_t;

/* TYPE_LANG_STRUCT: GCC internal language-specific types */
#ifdef ENABLE_CHECKING
struct GTY((for_user)) lang_struct_base {
    int code;
    union lang_struct_base* GTY((skip)) chain;
};
#endif

/* Vector types (often map to TYPE_LANG_STRUCT) */
typedef int GTY(()) v4si __attribute__((vector_size(16)));
typedef float GTY(()) v4sf __attribute__((vector_size(16)));

/* Complex type graph to ensure deep traversal */
struct GTY(()) type_graph_node {
    int node_id;
    
    /* Multiple pointer types */
    struct type_graph_node* GTY((skip)) self_ptr;
    struct opaque_struct* GTY((skip)) opaque_ptr;
    void* GTY((skip)) generic_ptr;
    
    /* Array types */
    int GTY(()) values[5];
    struct type_graph_node* GTY((skip)) neighbors[3];
    
    /* Union field */
    union {
        int as_int;
        float as_float;
        const char* as_string;
    } GTY((skip)) variant;
    
    /* Callback field */
    callback_func GTY((skip)) handler;
    
    /* Nested struct */
    struct {
        int depth;
        char label[32];
    } GTY((skip)) metadata;
};

/* More complex nested types */
union GTY(()) container_union {
    struct type_graph_node node;
    struct nested_struct nested;
    user_struct_t user;
};

struct GTY(()) master_container {
    int container_id;
    
    /* Array of different types */
    union container_union GTY(()) items[10];
    
    /* Pointer to array */
    int* GTY((length("%h.item_count"))) dynamic_array;
    int item_count;
    
    /* Callback array */
    callback_func GTY(()) handlers[5];
    
    /* String array */
    const char* GTY(()) names[8];
    
    /* Nested container */
    struct master_container* GTY((skip)) next_container;
};

/* Function pointer with complex signature */
typedef struct master_container* GTY((callback)) 
        (*factory_func)(int, const char**, callback_func*);

/* External declarations to create undefined types */
extern struct GTY(()) external_undefined;
extern union GTY(()) external_union_undefined;

/* Global variables with various types */
struct simple_struct GTY(()) global_struct = {1, 'a', 100};
union simple_union GTY(()) global_union = {.i = 42};
user_struct_t GTY(()) global_user_struct = {0, {.int_val = 100}};
v4si GTY(()) global_vector = {1, 2, 3, 4};

/* Array of mixed types */
void* GTY(()) mixed_array[] = {
    &global_struct,
    &global_union,
    &global_user_struct,
    "string element",
    NULL
};

/* Typedef chain leading to scalar */
typedef int GTY(()) base_int;
typedef base_int GTY(()) level1_int;
typedef level1_int GTY(()) level2_int;
typedef level2_int GTY(()) final_int;

/* Enum with bitfield */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    color_enum color : 3;
};

/* Self-referential types */
struct GTY(()) tree_node {
    int code;
    union {
        int int_val;
        double real_val;
        const char* string_val;
        struct tree_node* GTY((skip)) node_ptr;
    } GTY((desc("%0.code"))) u;
    struct tree_node* GTY((skip)) children[2];
};

/* Language-specific structure mimicking GCC internals */
struct GTY((for_user)) lang_specific {
    int lang_code;
    union {
        struct tree_node* ast_node;
        v4si vector_data;
        callback_func lang_callback;
    } GTY((desc("%0.lang_code"))) lang_data;
    struct lang_specific* GTY((skip)) next;
};

/* Complete the undefined types */
struct GTY(()) opaque_struct {
    int defined_now;
    void* GTY((skip)) data;
};

union GTY(()) opaque_union {
    int defined_now;
    struct opaque_struct* GTY((skip)) ptr;
};

/* Test function to ensure types are referenced */
void GTY(()) test_type_references(void) {
    /* Reference all types to ensure they're not optimized away */
    volatile int dummy = 0;
    
    dummy += sizeof(struct simple_struct);
    dummy += sizeof(union simple_union);
    dummy += sizeof(user_struct_t);
    dummy += sizeof(struct type_graph_node);
    dummy += sizeof(struct master_container);
    dummy += sizeof(v4si);
    dummy += sizeof(struct tree_node);
    dummy += sizeof(struct lang_specific);
    
    (void)dummy;
}
