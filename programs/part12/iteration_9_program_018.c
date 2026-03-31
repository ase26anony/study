/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage */
/* This file should be placed in the gcc/ directory and processed during GCC build */

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
char string_array[] = "hello world";

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct opaque_struct *opaque_ptr;
typedef const char *const_string_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*simple_callback)(void);
typedef struct my_struct *(*factory_fn)(int);

/* TYPE_ARRAY: Array types */
extern int incomplete_array[];
int fixed_size_array[10];
const char *string_ptr_array[5];
int *pointer_array[8];

/* TYPE_UNION: Union types */
union simple_union {
    int i;
    float f;
    void *p;
};

union complex_union {
    long l;
    double d;
    char *str;
    int array[4];
};

/* TYPE_STRUCT: Struct types with various fields */
struct simple_struct {
    int id;
    char name[32];
    float value;
};

struct nested_struct {
    int tag;
    union simple_union data;
    struct simple_struct *next;
};

struct recursive_struct {
    int value;
    struct recursive_struct *left;
    struct recursive_struct *right;
};

/* Now with GTY annotations for garbage collection */
/* These are crucial for gengtype to process the types */

/* TYPE_STRUCT with GTY */
struct GTY(()) gty_struct {
    int GTY((skip)) ignored_field;  /* skip this field for GC */
    scalar_int value;
    char *GTY((length("strlen(%h.name)+1"))) name;
    struct gty_struct *GTY((chain_next("%h.next"))) next;
};

/* TYPE_UNION with GTY */
union GTY(()) gty_union {
    int GTY((tag("0"))) as_int;
    float GTY((tag("1"))) as_float;
    void *GTY((tag("2"))) as_ptr;
    struct gty_struct *GTY((tag("3"))) as_struct;
};

/* TYPE_ARRAY with GTY */
typedef struct gty_struct *GTY((length("%0.count"))) gty_struct_array[];

/* TYPE_POINTER with GTY callback */
typedef int GTY((callback)) (*gty_callback_fn)(struct gty_struct *);

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* These typically require GCC-specific type definitions */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int GTY(()) v4si __attribute__((vector_size(16)));

/* Tree node-like structure (mimicking GCC internals) */
struct GTY(()) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

union GTY((desc("TREE_CODE(%0)"))) tree_node {
    struct tree_common common;
    /* In real GCC, there would be many more variants here */
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) type_graph_node {
    int id;
    
    /* TYPE_POINTER */
    struct type_graph_node **GTY((length("%h.neighbor_count"))) neighbors;
    int neighbor_count;
    
    /* TYPE_UNION */
    union {
        int int_data;
        float float_data;
        struct gty_struct *struct_data;
    } GTY((desc("%h.data_type"))) data;
    int data_type;
    
    /* TYPE_ARRAY */
    gty_callback_fn GTY((length("2"))) callbacks[2];
    
    /* TYPE_CALLBACK */
    int (*GTY((callback)) validate)(struct type_graph_node *);
};

/* External declarations to force TYPE_UNDEFINED handling */
extern struct undefined_external;
extern union undefined_union_external;

/* Function prototypes that use these types */
struct gty_struct *create_gty_struct(int value, const char *name);
void process_type_graph(struct type_graph_node *graph);
union tree_node *clone_tree_node(union tree_node *original);

/* Global variables with GTY markers */
extern struct gty_struct *GTY(()) global_gty_list;
extern union tree_node *GTY(()) global_tree_root;
extern v4si GTY(()) global_vector;

/* Nested type definitions for complex scenarios */
typedef struct {
    /* TYPE_STRUCT inside typedef */
    struct {
        int x;
        int y;
    } point;
    
    /* TYPE_UNION inside struct */
    union {
        int radius;
        struct {
            int width;
            int height;
        } rect;
    } shape;
    
    /* TYPE_CALLBACK inside */
    void (*GTY((callback)) draw)(void *);
} shape_descriptor;

/* Array of unions */
union variant {
    int i;
    float f;
    char *s;
} GTY(()) variant_array[10];

/* Pointer to array */
typedef int (*GTY((length("%h.size"))) int_array_ptr)[];

/* Self-referential structure with multiple pointer types */
struct GTY(()) complex_network {
    int id;
    
    /* Direct pointer */
    struct complex_network *GTY((refless)) parent;
    
    /* Array of pointers */
    struct complex_network **GTY((length("%h.child_count"))) children;
    int child_count;
    
    /* Pointer to callback */
    void (*GTY((callback)) notify)(struct complex_network *, int);
    
    /* Union containing pointers */
    union {
        struct gty_struct *gs;
        struct type_graph_node *tgn;
        void *generic;
    } GTY((desc("%h.data_kind"))) data;
    int data_kind;
};

#endif /* TEST_GTYPE_COVERAGE_H */
