/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in the gcc/ directory and processed during GCC build */

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

/* Scalar enum type */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_enum;

/* TYPE_STRING: String types */
const char GTY(()) *string_ptr = "test string";
char GTY(()) string_array[] = "array string";

/* TYPE_POINTER: Various pointer types */
typedef int* GTY(()) int_ptr;
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
union GTY(()) data_union {
    int i;
    float f;
    void* p;
    char* s;
};

/* TYPE_STRUCT: Regular struct types */
struct GTY(()) base_struct {
    int id;
    char* GTY((length("%h.name_len"))) name;
    size_t name_len;
};

/* Struct with nested union */
struct GTY(()) nested_struct {
    int type;
    union {
        int int_val;
        float float_val;
        char* GTY((tag("1"))) str_val;
    } GTY((desc("%0.type"))) value;
};

/* Recursive struct for chain_next testing */
struct GTY((chain_next("%h.next"))) linked_node {
    int data;
    struct linked_node* next;
};

/* Struct with array field */
struct GTY(()) array_container {
    int count;
    int GTY(()) values[8];
    int* GTY((length("%h.count"))) dynamic_values;
};

/* TYPE_USER_STRUCT: User-defined struct with special handling */
/* Using GCC vector extension to trigger special type handling */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

struct GTY(()) vector_struct {
    v4si vectors[2];
    int flags;
};

/* TYPE_LANG_STRUCT: GCC internal language structures */
/* Mimic tree_node structure from GCC internals */
struct GTY(()) tree_common {
    enum tree_code code;
    union tree_node* GTY((skip)) chain;
    location_t locus;
};

struct GTY(()) tree_type_common {
    struct tree_common common;
    union tree_node* GTY((skip)) values;
    unsigned int precision : 10;
    unsigned int no_force_blk : 1;
};

/* Complex type combining multiple type kinds */
struct GTY(()) master_container {
    /* Scalar fields */
    int id;
    enum color color;
    
    /* Pointer field */
    struct master_container* GTY((skip)) parent;
    
    /* Array field */
    struct linked_node* GTY((length("%h.node_count"))) nodes[5];
    int node_count;
    
    /* Union field */
    union {
        int int_data;
        struct {
            float x, y;
        } GTY((tag("2"))) point;
        char* GTY((tag("3"))) text;
    } GTY((desc("%0.data_type"))) data;
    int data_type;
    
    /* Callback field */
    compare_fn GTY((callback)) comparator;
    
    /* String field */
    const char* GTY((length("strlen(%h.message)+1"))) message;
    
    /* Nested struct */
    struct array_container container;
    
    /* Language struct field */
    struct tree_common* GTY((skip)) tree_node;
};

/* Array of pointers to structs */
struct master_container* GTY(()) container_array[4];

/* Union containing struct */
union GTY(()) struct_union {
    struct master_container container;
    struct linked_node node;
    int simple;
};

/* Function pointer returning struct */
struct master_container* GTY(()) (*factory_fn)(int) GTY((callback));

/* Typedef chain leading to scalar */
typedef int GTY(()) level1;
typedef level1 GTY(()) level2;
typedef level2 GTY(()) level3;

/* Opaque pointer typedef */
typedef struct opaque_struct* GTY(()) opaque_handle;

/* Self-referential structure */
struct GTY(()) self_ref {
    int value;
    struct self_ref* GTY((skip)) ref;
};

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
    int has_data;
    union {
        int* GTY((tag("1"))) int_ptr;
        char** GTY((tag("2"))) str_ptr;
    } GTY((desc("%0.has_data"))) data;
};

/* For testing TYPE_LANG_STRUCT more thoroughly */
/* Vector types often get special lang_struct handling */
typedef float GTY(()) v8sf __attribute__((vector_size(32)));
typedef double GTY(()) v4df __attribute__((vector_size(32)));

struct GTY(()) simd_container {
    v8sf floats;
    v4df doubles;
    int align;
};

/* Another lang_struct candidate: GCC's machine_mode */
typedef unsigned int GTY(()) machine_mode;

struct GTY(()) rtx_def {
    machine_mode mode : 8;
    unsigned int code : 8;
    union {
        int GTY((tag("0"))) int_val;
        const char* GTY((tag("1"))) str_val;
    } GTY((desc("%0.code"))) u;
};

/* Test variable declarations using these types */
struct master_container GTY(()) global_container;
union data_union GTY(()) global_union;
struct linked_node* GTY(()) global_list;
color_enum GTY(()) global_color = BLUE;

/* Array of unions */
union data_union GTY(()) union_array[5];

/* Pointer to array */
int (*GTY(()) array_ptr)[10];

/* Complex callback type */
typedef struct master_container* GTY((callback)) (*complex_callback)(
    struct master_container*,
    union data_union*,
    compare_fn
);

/* Nested callback */
complex_callback GTY((callback)) callback_holder;

/* Structure with all type kinds */
struct GTY(()) type_kitchen_sink {
    /* SCALAR */
    int scalar_int;
    color_enum scalar_enum;
    
    /* STRING */
    const char* string_ptr;
    char string_array[32];
    
    /* POINTER */
    void* void_ptr;
    int* int_ptr;
    struct type_kitchen_sink* self_ptr;
    
    /* ARRAY */
    int int_array[5];
    struct linked_node* ptr_array[3];
    
    /* UNION */
    union {
        int i;
        float f;
        void* p;
    } data_union;
    
    /* STRUCT */
    struct array_container nested_struct;
    
    /* CALLBACK */
    compare_fn callback;
    
    /* USER_STRUCT */
    v4si vector_data;
    
    /* LANG_STRUCT */
    struct tree_common* tree_data;
    
    /* Pointer to undefined type */
    struct opaque_struct* opaque_ptr;
};

/* Global instance for testing */
struct type_kitchen_sink GTY(()) test_instance;

/* Additional test for parameterized types */
struct GTY(()) template_like {
    void* GTY((skip)) data;
    size_t size;
    int (*GTY((callback)) compare)(const void*, const void*);
};

/* Chain of structures for traversal testing */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) double_link {
    int id;
    struct double_link* next;
    struct double_link* prev;
    char* GTY((length("%h.name_len"))) name;
    size_t name_len;
};

/* Root of the chain */
struct double_link* GTY(()) chain_root;

/* Union with struct and array */
union GTY(()) mega_union {
    struct master_container container;
    struct type_kitchen_sink sink;
    int int_array[16];
    void* ptr_array[8];
};

/* Final test structure that references everything */
struct GTY(()) final_test {
    struct master_container* main;
    union mega_union data;
    struct type_kitchen_sink* sink;
    struct double_link* chain;
    struct tree_common* tree;
    v4si vector;
    compare_fn cmp;
    const char* description;
    int counter;
    int GTY(()) numbers[4];
};
