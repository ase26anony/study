/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in the gcc/ directory and processed during build */

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
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String types */
const char *string_literal = "test string";
char string_array[] = "hello world";
typedef const char *string_ptr;

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct opaque_struct *opaque_ptr;
typedef int (*simple_func_ptr)(void);

/* TYPE_CALLBACK: Function pointer types with parameters */
typedef int (*comparator_callback)(const void *, const void *);
typedef void (*traverse_callback)(void *);
typedef size_t (*alloc_callback)(size_t);

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int incomplete_array[];
typedef int array_typedef[5];
typedef struct opaque_struct *ptr_array[8];

/* TYPE_UNION: Union types */
union simple_union {
    int i;
    float f;
    void *p;
};

union nested_union {
    int tag;
    union {
        int ival;
        float fval;
    } data;
    char *str;
};

/* TYPE_STRUCT: Struct types with various fields */
struct simple_struct {
    int id;
    char *name;
    struct simple_struct *next;
};

struct complex_struct {
    int counter;
    char buffer[256];
    union simple_union data;
    struct complex_struct *parent;
    struct simple_struct children[4];
    comparator_callback compare;
};

/* Recursive struct for chain_next testing */
struct GTY((chain_next("%h.next"))) linked_list {
    int value;
    struct linked_list *next;
};

/* Struct with array of pointers */
struct GTY(()) pointer_array_struct {
    int count;
    void * GTY((length("%h.count"))) items[];
};

/* TYPE_USER_STRUCT: User-defined struct with GTY markers and options */
struct GTY((user)) user_defined_struct {
    int magic;
    char * GTY((skip)) skip_field;  /* Skip this field for GC */
    struct complex_struct *nested;
};

/* TYPE_LANG_STRUCT: GCC internal/lang-specific struct types */
/* These typically use special GCC internal types or attributes */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

struct GTY(()) tree_int_cst {
    struct tree_common common;
    HOST_WIDE_INT val[2];
};

union GTY((desc("TREE_CODE(%0.code)"))) tree_node {
    struct tree_common common;
    struct tree_int_cst int_cst;
    /* Add more tree variants as needed */
};

/* RTL-like structure */
struct GTY(()) rtx_def {
    int code;
    union {
        HOST_WIDE_INT hwint;
        struct rtx_def *rtx;
        char *str;
    } u;
};

/* More complex type combinations to ensure full coverage */

/* Struct containing callback */
struct GTY(()) callback_container {
    traverse_callback traverse;
    void *user_data;
    alloc_callback allocator;
};

/* Union with struct and array */
union GTY(()) mixed_union {
    struct complex_struct cs;
    int array[10];
    void (*func)(void);
};

/* Typedef chain leading to scalar */
typedef int base_type;
typedef base_type level1_type;
typedef level1_type level2_type;
typedef level2_type final_scalar_type;

/* Array of unions */
union simple_union union_array[5];

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Function returning struct */
struct simple_struct (*func_returning_struct)(int);

/* Nested anonymous struct/union */
struct GTY(()) anonymous_container {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* Variable length struct */
struct GTY(()) vla_struct {
    int len;
    int data[1];  /* Actually variable length */
};

/* Opaque pointer typedef */
typedef struct opaque_struct *GTY((opaque)) opaque_handle;

/* Tagged union with descriptor */
union GTY((desc("%0.tag"))) tagged_union {
    int tag;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
};

/* For testing TYPE_NONE - this should not be reachable in normal operation,
   but we include diverse types to ensure all other cases are covered */

/* Global variables with various types for root marking */
struct simple_struct GTY((root)) global_struct;
union tree_node GTY((root)) *global_tree;
int_ptr GTY((root)) global_int_ptr;
callback_container GTY((root)) global_callback;

/* Function declarations using these types */
void process_struct(struct complex_struct *cs);
union simple_union get_union_value(int selector);
int compare_values(const void *a, const void *b);

/* Inline function to force type usage */
static inline void use_all_types(void) {
    struct simple_struct ss = {0};
    union simple_union su;
    int_ptr ip = NULL;
    array_ptr ap = NULL;
    final_scalar_type fs = 42;
    color_enum ce = GREEN;
    
    /* Use variables to avoid unused warnings */
    (void)ss;
    (void)su;
    (void)ip;
    (void)ap;
    (void)fs;
    (void)ce;
    (void)string_literal;
    (void)fixed_array;
    (void)global_struct;
}

/* Additional header to include for more coverage */
#ifdef TEST_EXTRA_TYPES
#include "test-gtype-extra.h"
#endif
