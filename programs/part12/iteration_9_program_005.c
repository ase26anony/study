/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage
   This file is designed to be included in GCC's source tree to ensure
   gengtype processes all type kinds during serialization. */

#ifndef TEST_GTYPE_COVERAGE_H
#define TEST_GTYPE_COVERAGE_H

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
const char *global_string = "test string";
char string_array[] = "initialized string";

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_callback)(const void *, const void *);
typedef void (*simple_callback)(void);

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct opaque_struct *opaque_ptr;
typedef comparator_callback callback_ptr;

/* TYPE_ARRAY: Array types */
extern int incomplete_array[];
int fixed_size_array[10];
int *pointer_array[5];
const char *string_ptr_array[] = {"one", "two", "three"};

/* TYPE_UNION: Union types */
union simple_union {
    int i;
    float f;
    void *p;
};

union nested_union {
    int int_val;
    union simple_union nested;
    int_ptr ptr_val;
};

/* TYPE_STRUCT: Struct types */
struct simple_struct {
    int a;
    char b;
    float c;
};

struct nested_struct {
    struct simple_struct inner;
    union simple_union data;
    int count;
};

/* Recursive struct with pointer for linked list */
struct recursive_struct {
    int value;
    struct recursive_struct *next;
    struct recursive_struct *prev;
};

/* Complex struct with all type kinds */
struct comprehensive_struct {
    /* Scalar fields */
    int id;
    color_enum color;
    
    /* String field */
    const char *name;
    
    /* Pointer fields */
    void_ptr data;
    int_ptr numbers;
    
    /* Array field */
    int scores[5];
    
    /* Union field */
    union simple_union variant;
    
    /* Nested struct */
    struct simple_struct nested;
    
    /* Callback */
    comparator_callback compare;
    
    /* Self-reference */
    struct comprehensive_struct *parent;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* Using GCC-specific attributes and vector types */

/* Vector type - often treated as lang_struct */
typedef int v4si __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

union tree_node {
    struct tree_common common;
    /* Other tree node variants would go here */
};

/* Now add GTY annotations for garbage collection */

/* GTY-annotated types to ensure processing by gengtype */

/* Simple GTY struct */
struct GTY(()) gty_simple {
    int value;
    char *name;
};

/* GTY struct with chain_next for linked list */
struct GTY((chain_next("%h.next"))) gty_linked_node {
    int id;
    struct gty_linked_node *next;
    struct gty_linked_node *prev;
};

/* GTY union */
union GTY(()) gty_union {
    int i;
    float f;
    struct gty_simple *gs;
};

/* GTY struct containing union */
struct GTY(()) gty_complex {
    int tag;
    union gty_union data;
    struct gty_linked_node *list;
    int GTY((length("%h.count"))) *dynamic_array;
    int count;
};

/* GTY callback type */
typedef int GTY((callback)) (*gty_callback)(struct gty_complex *);

/* GTY struct with callback */
struct GTY(()) gty_with_callback {
    gty_callback handler;
    void *user_data;
};

/* Array of GTY pointers */
struct GTY(()) gty_container {
    struct gty_simple *GTY((length("%h.size"))) items[10];
    int size;
};

/* Nested GTY structures for deep type graph */
struct GTY(()) gty_level1 {
    int val1;
    struct gty_level2 *level2;
};

struct GTY(()) gty_level2 {
    int val2;
    struct gty_level3 *level3;
    struct gty_level1 *parent;
};

struct GTY(()) gty_level3 {
    int val3;
    union gty_union data;
    struct gty_level1 *root;
};

/* Function pointer table */
typedef struct GTY(()) {
    const char *name;
    void (*func)(void);
} gty_function_entry;

gty_function_entry GTY((length("4"))) function_table[] = {
    {"init", 0},
    {"process", 0},
    {"cleanup", 0},
    {"report", 0}
};

/* Incomplete array of GTY structs */
extern struct gty_simple GTY((length("unknown"))) *incomplete_gty_array[];

/* Language-specific structure using attribute */
struct GTY(()) lang_specific {
    int mode;
    v4si vector_data;
    union tree_node *tree_node;
};

/* Comprehensive GTY type covering all cases */
struct GTY(()) master_gty_type {
    /* Scalars */
    int id;
    color_enum color;
    
    /* String */
    const char *description;
    
    /* Pointers */
    struct gty_simple *simple;
    void *opaque;
    
    /* Arrays */
    int fixed_array[8];
    struct gty_linked_node *GTY((length("%h.node_count"))) *node_array;
    int node_count;
    
    /* Unions */
    union gty_union choice;
    
    /* Nested structs */
    struct gty_complex complex;
    
    /* Callback */
    gty_callback callback;
    
    /* Self-reference */
    struct master_gty_type *sibling;
    
    /* Language struct */
    struct lang_specific lang_data;
    
    /* Vector type */
    v4si vector;
};

/* External declarations to create TYPE_UNDEFINED references */
extern struct undefined_external;
extern union undefined_external_union;

/* Typedef chain leading to scalar */
typedef int base_int;
typedef base_int derived_int;
typedef derived_int final_int;

/* Multiple levels of pointer typedefs */
typedef int ***triple_ptr;
typedef void (*complex_callback)(struct master_gty_type ***, int[][10]);

/* Enum with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    color_enum color_bits : 2;
};

/* Union with struct members */
union GTY(()) mixed_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        float r;
        float g;
        float b;
        float a;
    } color;
    v4si vector;
};

/* Finally, a top-level structure that references everything */
struct GTY(()) coverage_root {
    struct master_gty_type *master;
    struct gty_container *container;
    union mixed_union data;
    struct bitfield_struct flags;
    triple_ptr triple_pointer;
    complex_callback handler;
    struct undefined_external *undefined_ptr;  /* TYPE_UNDEFINED */
};

#endif /* TEST_GTYPE_COVERAGE_H */
