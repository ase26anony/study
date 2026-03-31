/* test-coverage.gt - Comprehensive test file to cover all TYPE_* cases in gengtype */

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

/* TYPE_POINTER: Pointer types */
typedef int* int_ptr;
typedef struct opaque_struct* opaque_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef const char* string_array[5];

/* TYPE_STRUCT: Plain C struct without GTY marker */
struct plain_struct {
    int field1;
    float field2;
    const char *name;
};

/* TYPE_USER_STRUCT: User-defined GC-aware structure */
struct GTY((user)) user_struct {
    void *data;
    int id;
    callback_type callback;
};

/* TYPE_UNION: Union type */
union my_union {
    int a;
    float b;
    void *c;
    const char *d;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GTY((tag("LANG"))) lang_specific {
    int lang_field;
    void *lang_data;
    enum my_enum lang_enum;
};

/* Complex nested structure to ensure deep processing */
struct GTY(()) complex_nested {
    /* TYPE_POINTER inside struct */
    struct opaque_struct* opaque_field;
    
    /* TYPE_ARRAY inside struct */
    int numbers[20];
    
    /* TYPE_UNION inside struct */
    union my_union data_union;
    
    /* TYPE_STRING inside struct */
    const char *description;
    
    /* TYPE_CALLBACK inside struct */
    callback_type handler;
    
    /* TYPE_SCALAR inside struct */
    enum my_enum status;
    
    /* Flexible array member (TYPE_ARRAY) */
    char flexible_array[];
};

/* Another GTY-marked struct with various field types */
struct GTY(()) another_gty_struct {
    /* Pointer to another GTY struct */
    struct complex_nested * GTY((skip)) nested;
    
    /* Array of pointers */
    void * GTY((skip)) ptr_array[8];
    
    /* String field */
    const char * GTY((length("strlen(%h.name) + 1"))) name;
    
    /* Scalar fields */
    int count;
    float value;
    
    /* Union field */
    union my_union data;
    
    /* Callback field */
    callback_type on_event;
};

/* Union containing GTY structures */
union GTY((desc("%1.type"))) tagged_union {
    struct GTY((tag("0"))) {
        int type;
        struct another_gty_struct *data;
    } type0;
    
    struct GTY((tag("1"))) {
        int type;
        struct user_struct *user_data;
    } type1;
    
    struct GTY((tag("2"))) {
        int type;
        struct lang_specific *lang_data;
    } type2;
};

/* Structure with chain of pointers */
struct GTY(()) linked_node {
    int value;
    struct linked_node * GTY((skip)) next;
    struct linked_node * GTY((skip)) prev;
};

/* Array of structures */
typedef struct linked_node node_array[100];

/* Structure with nested arrays */
struct GTY(()) matrix_container {
    int rows;
    int cols;
    double * GTY((length("%h.rows * %h.cols"))) matrix;
};

/* Test structure with all basic types */
struct GTY(()) all_types_struct {
    /* Scalars */
    char c;
    short s;
    int i;
    long l;
    float f;
    double d;
    enum my_enum e;
    
    /* Pointers */
    void *void_ptr;
    int *int_ptr;
    const char *string_ptr;
    
    /* Arrays */
    int int_array[5];
    char char_array[32];
    
    /* Nested structures */
    struct plain_struct plain;
    struct user_struct *user;
    
    /* Union */
    union my_union u;
    
    /* Callback */
    callback_type cb;
};

/* Forward declaration that will be defined later */
struct forward_declared;

/* Structure using forward declared type */
struct GTY(()) uses_forward {
    struct forward_declared * GTY((skip)) fwd_ptr;
    int id;
};

/* Definition of forward declared structure */
struct GTY(()) forward_declared {
    int value;
    struct uses_forward * GTY((skip)) back_ref;
};

/* Structure with conditional fields */
struct GTY(()) conditional_struct {
    int GTY((skip)) flag;
    union {
        int int_value;
        float float_value;
        void *ptr_value;
    } GTY((desc("%0.flag"))) data;
};

/* Root structure for GC */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) root_struct {
    struct root_struct *next;
    struct root_struct *prev;
    struct all_types_struct *data;
    struct tagged_union *variant;
    struct matrix_container *matrix;
    struct linked_node *list;
};
