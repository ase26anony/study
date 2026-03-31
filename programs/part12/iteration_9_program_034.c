/* test-gtype-coverage.c - Comprehensive type definitions for gengtype coverage */
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
const char *global_string = "Hello, gengtype!";
char string_array[] = "Test string array";

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
int fixed_array[10];
extern int incomplete_array[];
typedef int array_of_ptrs[5];
int multi_dim_array[3][4];

/* TYPE_UNION: Union types */
union simple_union {
    int i;
    float f;
    void *p;
};

union tagged_union {
    int type;
    struct {
        int x, y;
    } point;
    struct {
        float radius;
    } circle;
};

/* TYPE_STRUCT: Regular struct types */
struct simple_struct {
    int id;
    char name[32];
    struct simple_struct *next;
};

struct complex_struct {
    int tag;
    union {
        int int_val;
        float float_val;
        void *ptr_val;
    } data;
    struct complex_struct *children[5];
    comparator_fn compare;
};

/* Recursive struct for chain_next testing */
struct linked_node {
    int value;
    struct linked_node *GTY((skip)) next_skip;
    struct linked_node *GTY((chain_next("%h.next"))) next;
};

/* TYPE_USER_STRUCT: User-defined struct types with GTY markers */
struct GTY((user)) user_defined_struct {
    int user_id;
    char *user_name;
    struct user_defined_struct *GTY((skip)) sibling;
};

/* TYPE_LANG_STRUCT: GCC internal language-specific types */

/* Simulating tree node structure */
struct GTY((tag("TS_BASE"))) tree_base {
    int code;
    union tree_node *chain;
};

struct GTY((tag("TS_COMMON"))) tree_common {
    struct tree_base base;
    int mode;
};

union GTY((desc("%0.code"))) tree_node {
    struct tree_base GTY((tag("0"))) base;
    struct tree_common GTY((tag("1"))) common;
};

/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

struct GTY(()) vector_struct {
    v4si int_vector;
    v4sf float_vector;
};

/* More complex type combinations */

/* Struct containing array of function pointers */
struct callback_container {
    int count;
    simple_callback callbacks[10];
};

/* Union containing struct with pointer to array */
union nested_union {
    struct {
        int *data;
        int size;
    } array_info;
    struct {
        char *name;
        int value;
    } named_value;
};

/* Typedef chain leading to scalar */
typedef int base_int;
typedef base_int derived_int;
typedef derived_int final_int;

/* Struct with bitfields (scalar handling) */
struct bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
    int regular_field;
};

/* Global variables with various types for gengtype to process */
struct simple_struct global_struct = {0};
union simple_union global_union;
int_ptr global_int_ptr = NULL;
comparator_fn global_comparator = NULL;
array_of_ptrs global_array_of_ptrs;

/* Function declarations with callback parameters */
void register_callback(GTY((callback)) simple_callback cb);
int sort_array(void *base, size_t nmemb, size_t size, 
               GTY((callback)) comparator_fn compar);

/* Inline struct definition in parameter */
struct inline_param {
    int process(struct { int x; int y; } point);
};

/* Complete the forward declarations */
struct opaque_struct {
    int hidden_data;
    void *secret_ptr;
};

union opaque_union {
    long long_data;
    double double_data;
};

/* Main test structure that includes everything */
struct GTY(()) master_test_struct {
    /* Scalars */
    scalar_int int_field;
    color_enum color_field;
    scalar_bool bool_field;
    
    /* Strings */
    const char *GTY((length("%h.str_len"))) string_field;
    int str_len;
    
    /* Pointers */
    int_ptr int_ptr_field;
    void_ptr void_ptr_field;
    struct master_test_struct *self_ptr;
    
    /* Arrays */
    int int_array[20];
    struct simple_struct struct_array[5];
    
    /* Unions */
    union simple_union simple_union_field;
    union nested_union nested_union_field;
    
    /* Structs */
    struct complex_struct complex_field;
    struct linked_node *list_head;
    
    /* Callbacks */
    comparator_fn compare_field;
    
    /* Language-specific */
    union tree_node *tree_node_field;
    struct vector_struct vector_field;
    
    /* User struct */
    struct user_defined_struct *user_struct_field;
    
    /* Undefined (completed later) */
    struct opaque_struct *opaque_field;
    union opaque_union opaque_union_field;
    
    /* Bitfields */
    struct bitfield_struct bitfield_field;
};

/* Array of master structs */
struct master_test_struct GTY(()) test_array[3];

/* Union containing all major types */
union GTY((tag("%0.type"))) universal_union {
    int type;
    struct master_test_struct GTY((tag("1"))) as_struct;
    union simple_union GTY((tag("2"))) as_simple_union;
    int_ptr GTY((tag("3"))) as_pointer;
    color_enum GTY((tag("4"))) as_enum;
    comparator_fn GTY((tag("5"))) as_callback;
};

/* Final global containing union */
union universal_union GTY(()) global_universal;

/* Function to exercise callback type */
static int GTY((callback)) 
test_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

/* Initialize test data */
void init_test_data(void) {
    global_comparator = test_comparator;
    global_struct.id = 1;
    global_struct.name[0] = 'A';
    global_universal.type = 1;
}
