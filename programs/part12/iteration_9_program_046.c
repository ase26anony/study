/* test-gtype-coverage.c - Comprehensive type coverage for gengtype-state.cc
   This file should be placed in the gcc/ directory and processed by gengtype
   during GCC build to ensure coverage of all type serialization cases. */

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
const char *global_string = "Hello, gengtype!";
char string_array[] = "Test string";

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef const char *const_string_ptr;
typedef struct opaque_struct *opaque_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*simple_callback)(void);
typedef struct my_struct *(*factory_fn)(int);

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int incomplete_array[];
typedef int array_of_arrays[5][10];

/* TYPE_UNION: Union types */
union simple_union {
    int i;
    float f;
    void *p;
};

/* GTY-annotated union for garbage collection */
union GTY(()) gty_union {
    int ival;
    double dval;
    void *ptr;
};

/* TYPE_STRUCT: Regular struct types */
struct simple_struct {
    int a;
    char b;
    double c;
};

struct nested_struct {
    int id;
    struct simple_struct inner;
    struct nested_struct *next;
};

/* TYPE_USER_STRUCT: GTY-annotated struct with various options */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) linked_list {
    int value;
    struct linked_list * GTY((skip)) next;
    struct linked_list *prev;
};

struct GTY((for_user)) user_defined_struct {
    int tag;
    union {
        int ival;
        double dval;
        const char *sval;
    } data;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* Mimic tree node structure */
struct GTY(()) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

struct GTY(()) tree_int_cst {
    struct tree_common common;
    HOST_WIDE_INT int_cst[2];
};

/* Vector type using GCC extension (often treated as lang_struct) */
typedef int v4si __attribute__((vector_size(16)));

struct GTY(()) vector_struct {
    v4si data;
    int size;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) complex_node {
    int id;
    
    /* TYPE_POINTER */
    struct complex_node **children;
    
    /* TYPE_ARRAY */
    int values[5];
    
    /* TYPE_UNION */
    union {
        int int_data;
        double double_data;
        void *ptr_data;
    } payload;
    
    /* TYPE_CALLBACK */
    void (* GTY((callback)) cleanup)(struct complex_node *);
    
    /* TYPE_STRING */
    const char *name;
};

/* Array of pointers to structs */
struct GTY(()) node_array_container {
    struct complex_node * GTY((length("%h.count"))) nodes[];
    int count;
};

/* Union containing struct */
union GTY(()) container_union {
    struct complex_node node;
    struct node_array_container array;
    void *data;
};

/* Recursive type definition */
struct GTY((tag("0"))) variant_node {
    int tag;
    union GTY((desc("%1.tag"))) {
        struct GTY((tag("1"))) {
            int x;
            int y;
        } point;
        struct GTY((tag("2"))) {
            const char *text;
            int length;
        } string;
        struct GTY((tag("3"))) {
            struct variant_node *left;
            struct variant_node *right;
        } pair;
    } variant;
};

/* Function pointer with complex signature */
typedef struct variant_node *(*node_transformer)(
    struct variant_node *,
    void *context,
    int flags
);

/* Struct containing function pointer */
struct GTY(()) processor {
    const char *name;
    node_transformer transform;
    struct processor *next;
};

/* Incomplete array in struct */
struct GTY(()) flexible_struct {
    int count;
    double data[];
};

/* Multiple levels of indirection */
typedef struct complex_node ***node_ppp;

/* Enumeration type */
typedef enum gty_token {
    TOKEN_IDENT,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_EOF
} gty_token_t;

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 1;
    unsigned int value : 14;
    unsigned int : 0;  /* pad to next boundary */
    unsigned int extra : 8;
};

/* Union with struct and array */
union GTY(()) mixed_union {
    struct {
        int type;
        union {
            int ival;
            float fval;
        } data;
    } s;
    unsigned char bytes[8];
};

/* Callback type with GC awareness */
typedef void GTY((callback)) (*gc_aware_callback)(
    void *data,
    size_t size
);

/* Main container struct that references many types */
struct GTY(()) type_coverage_container {
    /* TYPE_SCALAR */
    int id;
    color_enum color;
    
    /* TYPE_STRING */
    const char *description;
    
    /* TYPE_POINTER */
    struct type_coverage_container *next;
    void *user_data;
    
    /* TYPE_ARRAY */
    int scores[10];
    
    /* TYPE_UNION */
    union {
        int int_option;
        const char *string_option;
        void *ptr_option;
    } option;
    
    /* TYPE_STRUCT */
    struct simple_struct base;
    
    /* TYPE_CALLBACK */
    gc_aware_callback cleanup;
    
    /* TYPE_USER_STRUCT */
    struct linked_list *items;
    
    /* TYPE_LANG_STRUCT */
    struct tree_common *tree_node;
    
    /* Complex nested type */
    struct complex_node *root;
    
    /* Array of pointers */
    struct processor *processors[5];
    
    /* Flexible array member */
    int extra_data[];
};

/* Global variables with GTY annotation */
GTY(()) struct type_coverage_container *global_container;
GTY(()) union container_union *global_union;
GTY(()) struct variant_node *global_variant;

/* Function declarations using these types */
struct type_coverage_container *create_container(int id);
void process_container(struct type_coverage_container *container);
void register_callback(gc_aware_callback cb);

/* Inline function to force type usage */
static inline void use_types(void) {
    /* Reference all types to ensure they're not optimized away */
    volatile int dummy = sizeof(struct opaque_struct) +
                        sizeof(union opaque_union) +
                        sizeof(scalar_int) +
                        sizeof(global_string) +
                        sizeof(int_ptr) +
                        sizeof(comparator_fn) +
                        sizeof(fixed_array) +
                        sizeof(union simple_union) +
                        sizeof(struct simple_struct) +
                        sizeof(struct linked_list) +
                        sizeof(struct tree_common) +
                        sizeof(v4si) +
                        sizeof(struct complex_node) +
                        sizeof(struct variant_node) +
                        sizeof(gty_token_t) +
                        sizeof(struct bitfield_struct) +
                        sizeof(union mixed_union);
    (void)dummy;
}
