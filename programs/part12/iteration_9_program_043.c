/* test-gtype-coverage.c - Comprehensive type coverage for gengtype-state.cc */
/* This file should be placed in the gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_struct;
union opaque_union;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int my_int;
typedef char my_char;
typedef long my_long;
typedef _Bool my_bool;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String types */
const char *global_string = "Hello, gengtype!";
char string_array[] = "Test string array";

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*simple_callback)(void);

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct opaque_struct *opaque_ptr;
typedef comparator_fn callback_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int incomplete_array[];
int *pointer_array[5];
const char *string_ptr_array[] = {"one", "two", "three"};

/* TYPE_UNION: Union types */
union simple_union {
    int i;
    float f;
    void *p;
};

union nested_union {
    int tag;
    union {
        int int_val;
        float float_val;
    } data;
    char str[20];
};

/* TYPE_STRUCT: Struct types */
struct simple_struct {
    int id;
    char name[50];
    struct simple_struct *next;
};

struct complex_struct {
    int type;
    union {
        int int_val;
        float float_val;
        char *str_val;
    } data;
    struct complex_struct *children[5];
    comparator_fn compare;
};

/* Recursive struct for linked list */
struct linked_list {
    int value;
    struct linked_list *next;
    struct linked_list *prev;
};

/* Struct containing arrays */
struct array_container {
    int numbers[100];
    char *strings[10];
    struct simple_struct structs[5];
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* These typically require GTY annotations and special handling */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int v4si __attribute__((vector_size(16)));

/* Tree node-like structure mimicking GCC internals */
struct tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

struct tree_int_cst {
    struct tree_common common;
    long int_cst_low;
    long int_cst_high;
};

union tree_node {
    struct tree_common common;
    struct tree_int_cst int_cst;
    /* Add more tree types as needed */
};

/* Now with GTY annotations for garbage collection */

/* GTY-marked types for TYPE_STRUCT */
struct GTY(()) gty_struct {
    int GTY((skip)) ignored_field;  /* Skip this field in GC */
    char *GTY((tag("0"))) name;
    struct gty_struct *GTY((chain_next("%h.next"))) next;
    union gty_union *GTY((desc("%1.type"))) data;
};

/* GTY-marked union */
union GTY((desc("%0.type"))) gty_union {
    int type;
    struct {
        int int_val;
        float float_val;
    } GTY((tag("1"))) numeric;
    struct {
        char *str;
        int len;
    } GTY((tag("2"))) string;
};

/* GTY-marked pointer type */
typedef struct gty_struct *GTY(()) gty_struct_ptr;

/* GTY-marked array */
struct GTY(()) gty_array_container {
    gty_struct_ptr GTY((length("%h.count"))) items[10];
    int count;
};

/* GTY-marked callback type */
typedef int GTY((callback)) (*gty_comparator)(const void *, const void *);

/* Complex nested GTY structure */
struct GTY(()) complex_gty_struct {
    int id;
    union gty_union data;
    struct complex_gty_struct *GTY((reorder("complex_gty_struct_q"))) siblings[3];
    gty_comparator cmp_func;
    char *GTY((atomic)) atomic_string;  /* Atomic pointer */
    
    /* Nested anonymous struct */
    struct {
        int x;
        int y;
    } position;
    
    /* Variable length array at end */
    int GTY((variable_length)) flexible_array[];
};

/* TYPE_LANG_STRUCT examples using GCC extensions */
/* These might be recognized as language-specific types */

/* Vector type with GTY */
typedef int GTY(()) v2si __attribute__((vector_size(8)));

/* Transactional memory type */
struct GTY(()) tm_clone {
    int tm_id;
    void *tm_data;
} __attribute__((transaction_safe));

/* Target-specific type */
struct GTY(()) machine_insn {
    int opcode;
    int operands[3];
} __attribute__((target("arch=x86-64")));

/* For TYPE_USER_STRUCT - user-defined type with special handling */
/* This would typically be defined in a language frontend */

#ifdef GENERATOR_FILE
/* Types only visible to gengtype */
struct generator_only_struct {
    int magic;
    char *generator_data;
};

union generator_only_union {
    int code;
    struct generator_only_struct *data;
};
#endif

/* Function declarations using these types */
void process_struct(struct simple_struct *s);
union simple_union create_union(int type);
int compare_values(const void *a, const void *b);
struct complex_gty_struct *create_complex_gty(int id);

/* Global variables with various types */
struct simple_struct global_struct = {1, "global", NULL};
union simple_union global_union = {.i = 42};
color_enum global_color = GREEN;
gty_struct_ptr global_gty_ptr = NULL;
v4si global_vector = {1, 2, 3, 4};

/* Array of different types */
void *type_array[] = {
    &global_struct,
    &global_union,
    &global_color,
    global_string,
    (void*)compare_values
};

/* Incomplete array definition */
int incomplete_array[20];  /* Now defined */

/* Main function to ensure the file compiles */
int main(void) {
    struct simple_struct local_struct = {2, "local", &global_struct};
    union simple_union local_union = {.f = 3.14f};
    
    /* Use all types to avoid compiler warnings */
    int_ptr ptr = &local_struct.id;
    void_ptr vptr = &local_union;
    
    fixed_array[0] = 1;
    incomplete_array[0] = 2;
    
    return 0;
}

/* Additional type definitions in different scopes */

/* File-scoped types */
static struct static_struct {
    int private_data;
    char private_name[20];
} static_instance;

/* Externally visible types for multiple translation units */
extern struct exported_struct {
    int public_field;
    union simple_union public_union;
} exported_instance;

/* Typedef chains leading to scalar types */
typedef int base_type;
typedef base_type derived_type;
typedef derived_type final_type;

/* Function pointer with complex signature */
typedef void (*complex_callback)(
    struct complex_gty_struct *,
    union gty_union *,
    int,
    ...
);

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Union with bitfields */
union bitfield_union {
    unsigned int all_flags;
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } parts;
};

/* For maximum coverage, also define types that might be processed
   during different phases of gengtype execution */

/* Preprocessor-controlled types */
#if defined(ENABLE_ADVANCED_TYPES)
struct advanced_type {
    /* Complex type graph */
    struct advanced_type **ref_chain;
    void (*destructor)(struct advanced_type *);
};

/* Self-referential type with multiple pointers */
struct graph_node {
    int id;
    struct graph_node **neighbors;
    int neighbor_count;
};
#endif

/* End of comprehensive type definitions */
