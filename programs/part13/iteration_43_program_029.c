/* test-coverage.gt - Comprehensive test file for gengtype coverage */

/* TYPE_UNDEFINED: Forward declaration of opaque struct */
struct opaque_struct;

/* TYPE_SCALAR: Basic scalar types and enums */
typedef int scalar_int;
typedef float scalar_float;
enum my_enum { E1, E2, E3 };

/* TYPE_STRING: String type */
typedef const char *string_type;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_type)(int);
typedef int (*another_callback)(const char*, float);

/* TYPE_POINTER: Various pointer types */
typedef int* int_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
int fixed_array[10];
typedef char char_array[20];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    char field3;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    char d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
};

/* Complex nested types to ensure deep processing */

/* A GC-tracked struct containing various types */
struct GTY(()) complex_struct {
    /* TYPE_POINTER */
    void *ptr_field;
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_ARRAY (fixed size) */
    int scores[5];
    
    /* TYPE_CALLBACK */
    callback_type callback;
    
    /* TYPE_UNION */
    union my_union data_union;
    
    /* TYPE_SCALAR */
    enum my_enum status;
    
    /* Pointer to another GC type */
    struct GTY(()) inner_struct *inner;
    
    /* Flexible array member (variable-length array) */
    int flexible_array[];
};

/* Another GC struct for additional coverage */
struct GTY(()) inner_struct {
    int id;
    string_type description;
    
    /* Array of pointers */
    void * GTY((length("%0.array_len"))) *ptr_array;
    int array_len;
    
    /* Nested struct */
    struct plain_struct nested_plain;
};

/* Union with GTY marker */
union GTY((desc("%1.type"))) tagged_union {
    int type;
    struct complex_struct * GTY((tag("0"))) as_complex;
    struct inner_struct * GTY((tag("1"))) as_inner;
};

/* Type with callback field */
struct GTY(()) callback_container {
    callback_type handler;
    void *user_data;
    int (* GTY((skip)) ignored_callback)(void); /* Skip this for GC */
};

/* Additional coverage for edge cases */

/* Array of structs */
struct GTY(()) array_of_structs {
    struct inner_struct elements[3];
};

/* Pointer to array */
typedef int (*array_ptr)[10];

/* Const pointer */
typedef const int *const_int_ptr;

/* Struct with bitfields (scalar type) */
struct bitfield_struct {
    unsigned int flag1:1;
    unsigned int flag2:2;
    unsigned int flag3:3;
};

/* Opaque pointer type */
typedef struct opaque_struct *opaque_handle;

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node * GTY((skip)) left;  /* Skip to avoid infinite recursion in test */
    struct tree_node * GTY((skip)) right; /* Skip to avoid infinite recursion in test */
};

/* Structure with language-specific chain_next field */
struct GTY((chain_next("%h.next"))) chainable_struct {
    int data;
    struct chainable_struct *next;
};

/* Test TYPE_NONE - This should not be directly triggerable as it's for internal error handling */
/* The case TYPE_NONE: gcc_unreachable(); is for internal consistency checking */

/* Additional typedefs for coverage */
typedef struct complex_struct complex_t;
typedef union tagged_union variant_t;

/* Global variables of various types */
int GTY((skip)) global_scalar = 42;  /* Skip non-pointer global */
struct complex_struct * GTY((root)) global_root;  /* Root for GC */
const char * GTY((length("strlen(%h.name)+1"))) global_name;
