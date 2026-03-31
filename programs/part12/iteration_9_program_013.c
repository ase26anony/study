/* test-gtype-coverage.c - A test file to cover all gengtype type cases */
/* This file should be placed in gcc/ directory and processed during GCC build */

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
const char *global_string = "Hello, GCC!";
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

union complex_union {
    struct { int x; int y; } point;
    struct { char *name; int id; } info;
    double value;
};

/* TYPE_STRUCT: Regular struct types */
struct simple_struct {
    int id;
    char *name;
    struct simple_struct *next;
};

struct nested_struct {
    int data;
    union simple_union u;
    struct {
        int x;
        int y;
    } coord;
    char buffer[50];
};

/* Recursive struct for chain_next testing */
struct linked_node {
    int value;
    struct linked_node *GTY((skip)) next_skip;
    struct linked_node *GTY((chain_next("%h.next"))) next;
};

/* Struct with array field */
struct array_container {
    int count;
    int items[20];
    struct array_container *GTY((skip)) parent;
};

/* GTY-annotated types for garbage collection */

/* TYPE_STRUCT with GTY */
struct GTY(()) gty_struct {
    int GTY((tag("0"))) tag_field;
    union simple_union GTY((desc("%1.tag_field"))) u;
    char *GTY((length("%h.name_len"))) name;
    int name_len;
    struct gty_struct *GTY((chain_next("%h.next_ptr"))) next_ptr;
};

/* TYPE_UNION with GTY */
union GTY((desc("%0.kind"))) gty_union {
    int GTY((tag("0"))) kind;
    struct {
        int x;
        int y;
    } GTY((tag("1"))) point;
    struct {
        char *text;
        int length;
    } GTY((tag("2"))) string;
};

/* TYPE_ARRAY with GTY in struct context */
struct GTY(()) array_wrapper {
    int GTY((length("%h.count"))) *dynamic_array;
    int count;
    union gty_union GTY((skip)) items[10];
};

/* TYPE_POINTER with callback attribute */
typedef int GTY((callback)) (*gty_callback)(struct gty_struct *);

/* Struct containing callback */
struct GTY(()) callback_container {
    gty_callback cb;
    void *GTY((skip)) user_data;
};

/* TYPE_LANG_STRUCT: GCC-specific types */
/* Vector types using GCC extension */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Tree-like structure mimicking GCC internals */
struct GTY(()) tree_common {
    int code;
    union tree_node *chain;
    union tree_node *type;
};

union GTY((desc("%0.code"))) tree_node {
    struct tree_common common;
    struct {
        struct tree_common common;
        const char *pointer;
    } GTY((tag("1"))) identifier;
    struct {
        struct tree_common common;
        long long int int_cst;
    } GTY((tag("2"))) integer;
};

/* TYPE_USER_STRUCT: User-defined struct type with special handling */
/* Using a struct with nested anonymous structs and unions */
struct GTY(()) user_defined {
    int type_id;
    union {
        struct {
            int x, y;
        } point;
        struct {
            char *data;
            int len;
        } buffer;
    } GTY((desc("%1.type_id"))) value;
    
    /* Array of pointers with length */
    struct user_defined *GTY((length("%h.ref_count"))) *references;
    int ref_count;
};

/* Complex nested type to ensure deep traversal */
struct GTY(()) outer_container {
    struct gty_struct *inner_struct;
    union gty_union current_union;
    struct array_wrapper wrapper;
    struct callback_container callbacks[5];
    struct user_defined *user_data;
    
    /* Multi-dimensional array */
    int matrix[3][3];
    
    /* Pointer to array */
    int (*func_ptr_array[3])(int, int);
    
    /* Nested anonymous struct */
    struct {
        int depth;
        struct outer_container *GTY((skip)) parent;
    } metadata;
};

/* Function pointer type definitions for various signatures */
typedef struct gty_struct* (*factory_fn)(int);
typedef void (*destructor_fn)(struct outer_container*);
typedef union gty_union (*processor_fn)(const char*);

/* Global variables with GTY annotations */
struct gty_struct * GTY((root)) global_root = NULL;
union tree_node * GTY((root)) global_tree = NULL;
struct outer_container * GTY((chain_next("%h.next_global"))) global_chain = NULL;

/* Static variables */
static struct linked_node *static_list = NULL;
static union simple_union static_unions[5];

/* Inline struct definition */
struct inline_example {
    int counter;
    struct {
        short a;
        short b;
    } pair;
    void (*operation)(struct inline_example*);
};

/* Typedef chain leading to scalar */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 final_int;

/* Enum with explicit values */
enum gty_flags {
    FLAG_NONE = 0,
    FLAG_READ = 1 << 0,
    FLAG_WRITE = 1 << 1,
    FLAG_EXECUTE = 1 << 2,
    FLAG_ALL = FLAG_READ | FLAG_WRITE | FLAG_EXECUTE
};

/* Struct using the enum */
struct GTY(()) flag_container {
    enum gty_flags flags;
    char *name;
};

/* Complete the forward declarations */
struct opaque_struct {
    int hidden_data;
    struct opaque_struct *next;
};

union opaque_union {
    int as_int;
    struct opaque_struct *as_ptr;
};

/* Array of incomplete type */
extern struct incomplete_type;
struct incomplete_type *unknown_array[5];

/* Const pointer types */
typedef const char *const_string_ptr;
typedef const struct gty_struct *const_struct_ptr;

/* Volatile types */
typedef volatile int volatile_int;
typedef volatile struct linked_node *volatile_node_ptr;

/* Struct with bitfield */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Union with bitfield */
union bitfield_union {
    struct {
        unsigned int low : 16;
        unsigned int high : 16;
    } parts;
    unsigned int whole;
};

/* Make sure all types are referenced to avoid optimization */
void reference_all_types(void) {
    /* Reference each type to ensure they're not optimized away */
    volatile int dummy = 0;
    
    /* Force references */
    dummy += sizeof(struct opaque_struct);
    dummy += sizeof(union opaque_union);
    dummy += sizeof(scalar_int);
    dummy += sizeof(color_enum);
    dummy += sizeof(global_string[0]);
    dummy += sizeof(string_array[0]);
    dummy += sizeof(comparator_fn);
    dummy += sizeof(int_ptr);
    dummy += sizeof(fixed_array[0]);
    dummy += sizeof(union simple_union);
    dummy += sizeof(struct simple_struct);
    dummy += sizeof(struct nested_struct);
    dummy += sizeof(struct linked_node);
    dummy += sizeof(struct gty_struct);
    dummy += sizeof(union gty_union);
    dummy += sizeof(struct array_wrapper);
    dummy += sizeof(gty_callback);
    dummy += sizeof(struct callback_container);
    dummy += sizeof(v4si);
    dummy += sizeof(struct tree_common);
    dummy += sizeof(union tree_node);
    dummy += sizeof(struct user_defined);
    dummy += sizeof(struct outer_container);
    dummy += sizeof(factory_fn);
    dummy += sizeof(struct inline_example);
    dummy += sizeof(final_int);
    dummy += sizeof(enum gty_flags);
    dummy += sizeof(struct flag_container);
    dummy += sizeof(const_string_ptr);
    dummy += sizeof(volatile_int);
    dummy += sizeof(struct bitfield_struct);
    dummy += sizeof(union bitfield_union);
    
    /* Prevent unused variable warning */
    (void)dummy;
}
