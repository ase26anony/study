/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_undefined;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef long scalar_long;
typedef _Bool scalar_bool;
typedef enum { RED, GREEN, BLUE } scalar_enum;

/* TYPE_STRING: String types */
const char *string_ptr = "test string";
char string_array[] = "hello world";
typedef const char *string_type;

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef void *void_ptr;
typedef struct opaque_undefined *opaque_ptr;
typedef const char *const *double_ptr;

/* TYPE_CALLBACK: Function pointer types */
typedef int (*callback_simple)(void);
typedef void (*callback_complex)(int, char *);
typedef int (*callback_compare)(const void *, const void *);
typedef struct my_struct *(*callback_return_struct)(void);

/* TYPE_ARRAY: Array types */
int fixed_array[10];
extern int incomplete_array[];
typedef int array_typedef[5];
typedef struct my_struct *ptr_array[8];

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
    struct my_struct *s;
};

/* TYPE_STRUCT: Regular struct types */
struct simple_struct {
    int a;
    char b;
    long c;
};

struct nested_struct {
    int id;
    struct simple_struct inner;
    struct nested_struct *next;
};

struct complex_struct {
    int tag;
    union {
        int i;
        float f;
        char *s;
    } data;
    struct complex_struct *children[4];
    callback_compare compare_fn;
};

/* GTY-annotated types for garbage collection */
struct GTY(()) gty_struct {
    int GTY((skip)) ignored_field;
    struct gty_struct *GTY((tag("0"))) next;
    char *GTY((length("strlen(%h.data) + 1"))) data;
    union complex_union GTY((desc("%1.tag"))) variant;
};

union GTY(()) gty_union {
    struct gty_struct *GTY((tag("1"))) s;
    int GTY((tag("2"))) i;
    callback_simple GTY((tag("3"))) fn;
};

typedef struct gty_struct *GTY((ptr)) gty_ptr;

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
struct GTY((user)) user_struct {
    int magic;
    void *GTY((skip)) opaque_data;
    struct user_struct *GTY((chain)) chain_next;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific types */
/* These typically come from tree.h or rtl.h in GCC */

/* Simulating tree_node-like structure */
struct GTY((lang_struct)) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

union GTY((lang_struct)) tree_node {
    struct tree_common common;
    /* In real GCC, there would be many more variants here */
    struct {
        int int_value;
    } integer;
};

/* Vector types using GCC extensions (often map to lang_struct) */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

struct GTY(()) vector_struct {
    v4si int_vec;
    v4sf float_vec;
};

/* Complex type graph to ensure deep traversal */
struct GTY(()) type_graph_root {
    struct gty_struct *first;
    union gty_union *variants[8];
    callback_complex processor;
    struct type_graph_root *GTY((reorder("resort_type_graph_root"))) next;
    
    /* Nested anonymous struct */
    struct {
        int counter;
        char name[32];
    } metadata;
};

/* Array of pointers with callback elements */
typedef void (*action_callback)(struct type_graph_root *);
action_callback GTY((length("%h.count"))) callbacks[];

/* Recursive type definition */
typedef struct recursive_node {
    int value;
    struct recursive_node *left;
    struct recursive_node *right;
    void (*visit)(struct recursive_node *);
} recursive_node_t;

/* Opaque pointer type for undefined struct */
typedef struct undefined_struct *opaque_handle;

/* Enumeration with many values */
typedef enum gty_token {
    TOKEN_EOF,
    TOKEN_IDENT,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_MAX
} gty_token_t;

/* Struct with bitfields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Union with nested struct */
union GTY(()) nested_union {
    struct {
        int x;
        int y;
    } point;
    struct {
        int width;
        int height;
    } size;
    int array[4];
};

/* Type with array of callbacks */
struct GTY(()) callback_container {
    int count;
    int (*handlers[10])(void *data);
    void *user_data;
};

/* Forward declaration that will be defined later */
struct later_defined;
struct later_defined {
    int value;
    struct later_defined *next;
};

/* Typedef chain leading to scalar */
typedef int base_type;
typedef base_type middle_type;
typedef middle_type final_type;

/* Struct with all type kinds as members */
struct GTY(()) all_types_container {
    /* SCALAR */
    int integer;
    gty_token_t token;
    
    /* POINTER */
    void *opaque;
    struct gty_struct *gty_ptr;
    
    /* ARRAY */
    int numbers[5];
    char name[32];
    
    /* STRING */
    const char *message;
    
    /* CALLBACK */
    callback_compare comparator;
    
    /* UNION */
    union complex_union value;
    
    /* Nested STRUCT */
    struct simple_struct nested;
    
    /* Reference to USER_STRUCT */
    struct user_struct *user;
    
    /* Reference to LANG_STRUCT */
    union tree_node *tree;
    
    /* Function pointer array */
    void (*actions[3])(void);
    
    /* Pointer to undefined type */
    struct opaque_undefined *undefined;
};

/* Global variables to ensure types are used */
struct type_graph_root *GTY((root)) global_root = NULL;
union tree_node *GTY((root)) global_tree = NULL;
struct all_types_container GTY((root)) global_container;

/* Callback function definitions */
static int simple_callback(void) {
    return 42;
}

static void complex_callback(int x, char *s) {
    /* Do nothing */
}

/* Initialization function */
void init_type_coverage(void) {
    static struct all_types_container init_container = {
        .integer = 1,
        .token = TOKEN_IDENT,
        .message = "initialized",
        .numbers = {1, 2, 3, 4, 5},
        .name = "test"
    };
    
    global_container = init_container;
    global_container.comparator = (callback_compare)strcmp;
}
