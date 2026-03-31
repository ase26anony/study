/* test-gtype-coverage.h - Comprehensive type declarations for gengtype coverage
   This file contains diverse type declarations to trigger all type serialization
   cases in write_state_type() function in gengtype-state.cc */

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
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String types */
const char *global_string = "Hello, gengtype!";
char string_array[] = "Test string array";
typedef const char *string_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*simple_callback)(void);
typedef struct my_struct *(*struct_factory)(int, char);

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct my_struct *struct_ptr;
typedef union my_union *union_ptr;
typedef int (*func_ptr)(void);
typedef int (*complex_func_ptr)(int, char *, struct my_struct **);

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int incomplete_array[];
typedef int array_typedef[5];
typedef char string_literal_array[32];

/* TYPE_UNION: Union types */
union my_union {
    int i;
    float f;
    void *p;
    char *str;
};

union nested_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        float start;
        float end;
    } range;
    void *data;
};

/* TYPE_STRUCT: Struct types with various fields */
struct my_struct {
    int id;
    char *name;
    struct my_struct *next;
    union my_union data;
    int scores[5];
    comparator_fn compare;
};

struct complex_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        char *string_val;
        struct my_struct *struct_val;
    } value;
    struct complex_struct *left;
    struct complex_struct *right;
    void (*processor)(struct complex_struct *);
};

/* Recursive struct for chain_next testing */
struct linked_list {
    int data;
    struct linked_list *GTY((chain_next("%h.next"))) next;
};

/* TYPE_USER_STRUCT / TYPE_LANG_STRUCT: GCC internal types */
/* Using GCC-specific attributes and extensions */

/* Vector type - may be treated as lang_struct */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

union tree_node {
    struct tree_common common;
    struct tree_decl decl;
    struct tree_type type;
};

struct tree_decl {
    struct tree_common common;
    char *name;
    union tree_node *type;
};

struct tree_type {
    struct tree_common common;
    int precision;
    int align;
};

/* More complex GCC-like structures */
struct gimple_statement_base {
    int code;
    unsigned num_ops;
    struct tree_node *ops[1];
};

struct rtx_def {
    int code;
    int mode;
    union {
        int int_val;
        char *str;
        struct rtx_def *rtx;
    } u;
};

/* GTY-annotated types for garbage collection */

/* Simple GTY struct */
struct GTY(()) gty_simple {
    int value;
    char *GTY((skip)) name;  /* skip in GC */
    struct gty_simple *next;
};

/* GTY struct with callback */
struct GTY(()) gty_with_callback {
    int id;
    int (*GTY((callback))) processor(int, void *);
    struct gty_with_callback *GTY((chain_next("%h.next"))) next;
};

/* GTY union */
union GTY(()) gty_union {
    int i;
    float f;
    struct gty_simple *gs;
    char *str;
};

/* GTY array */
struct GTY(()) gty_array_container {
    int count;
    struct gty_simple *GTY((length("%h.count"))) items[];
};

/* Complex nested GTY structure */
struct GTY(()) gty_complex {
    enum { TAG_INT, TAG_STRING, TAG_STRUCT } tag;
    union {
        int int_value;
        char *string_value;
        struct gty_simple *struct_value;
    } data;
    
    /* Array of pointers */
    struct gty_simple **GTY((length("5"))) ptr_array;
    
    /* Callback function */
    void (*GTY((callback))) cleanup(struct gty_complex *);
    
    /* For chain traversal */
    struct gty_complex *GTY((chain_next("%h.next"))) next;
    struct gty_complex *prev;
};

/* Function pointer typedef with GTY */
typedef void GTY((callback)) (*gty_callback_fn)(void *data, int result);

/* Struct using the callback typedef */
struct GTY(()) gty_with_typedef_callback {
    int state;
    gty_callback_fn callback;
    void *user_data;
};

/* Incomplete array in GTY struct */
struct GTY(()) gty_incomplete_array {
    int len;
    char data[];
};

/* Self-referential union */
union GTY(()) self_ref_union {
    int type;
    struct {
        union self_ref_union *GTY((skip)) next;
        char *name;
    } link;
    struct {
        int value;
        union self_ref_union *children[2];
    } node;
};

/* Multiple indirection types */
typedef struct gty_complex ***complex_ptr_ptr;
typedef int (*array_of_funcs[5])(void);

/* Enum with many values for scalar coverage */
typedef enum {
    STATE_INIT,
    STATE_PROCESSING,
    STATE_WAITING,
    STATE_COMPLETE,
    STATE_ERROR,
    STATE_MAX
} process_state;

/* Struct with bitfields (scalar handling) */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Union with bitfields */
union bitfield_union {
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : 4;
        unsigned int d : 4;
    } parts;
    unsigned int whole;
};

/* Opaque pointer typedef */
typedef struct undefined_struct *opaque_handle;

/* Function returning struct by value */
struct small_struct {
    int x, y;
};

struct small_struct make_small_struct(int x, int y);

/* Array of unions */
union data_cell {
    int i;
    float f;
    void *p;
};

union data_cell data_grid[10][10];

/* Struct with flexible array member */
struct flex_array {
    int count;
    union data_cell items[];
};

/* Typedef chain leading to scalar */
typedef int base_int;
typedef base_int level1_int;
typedef level1_int level2_int;
typedef level2_int final_int;

/* Complex function pointer with struct parameter */
struct callback_context {
    int id;
    void *data;
};

typedef int (*complex_callback)(
    struct callback_context *ctx,
    const char *input,
    char **output,
    size_t *output_len
);

/* Global variables with various types for gengtype to process */
extern struct my_struct *global_struct_list;
extern union my_union global_union_var;
extern int (*global_callback)(void);
extern char *global_string_array[];
extern struct GTY(()) gty_complex *global_gty_complex;

/* Inline function using the types */
static inline void process_types(void) {
    /* Reference all types to ensure they're not optimized away */
    volatile int dummy = 0;
    
    struct my_struct local_struct = {0};
    union my_union local_union;
    struct gty_simple *local_gty = 0;
    
    dummy += sizeof(struct opaque_struct);
    dummy += sizeof(color_enum);
    dummy += sizeof(comparator_fn);
    dummy += sizeof(v4si);
    dummy += sizeof(struct tree_common);
    
    (void)dummy;
    (void)local_struct;
    (void)local_union;
    (void)local_gty;
}

#endif /* TEST_GTYPE_COVERAGE_H */
