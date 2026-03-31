/* Test header to cover all gengtype-state.cc switch cases */

/* TYPE_UNDEFINED - forward declaration creates undefined type */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR - fundamental scalar type */
extern GTY(()) int global_scalar;

/* TYPE_STRING - string type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK - function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_ARRAY - fixed-size array */
typedef int GTY(()) int_array[10];
extern GTY(()) int_array global_array;

/* TYPE_POINTER - pointer type */
struct GTY(()) base_struct {
    int id;
    struct base_struct* GTY((skip)) next;
};
typedef struct base_struct* GTY(()) base_ptr;
extern GTY(()) base_ptr global_base_ptr;

/* TYPE_STRUCT - plain C struct */
struct GTY(()) my_struct {
    int field1;
    double field2;
    base_ptr GTY((tag("0"))) ptr_field;
    int_array array_field;
};

/* TYPE_USER_STRUCT - struct with user-defined marking */
struct GTY((user)) user_struct {
    void* GTY((skip)) data;
    int data_size;
};

/* TYPE_UNION - union type */
union GTY(()) my_union {
    int i;
    double d;
    base_ptr p;
    struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_LANG_STRUCT - language-specific structure */
enum test_node_codes {
    TEST_NODE_TYPE1,
    TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
    enum test_node_codes code;
    union GTY((desc("%1.code"))) {
        struct GTY((tag("0"))) {
            int int_val;
        } type1;
        struct GTY((tag("1"))) {
            double double_val;
            struct lang_struct* GTY((skip("0"))) child;
        } type2;
    } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
    /* TYPE_STRUCT member */
    struct my_struct nested_struct;
    
    /* TYPE_UNION member */
    union my_union nested_union;
    
    /* TYPE_ARRAY of pointers */
    base_ptr GTY(()) ptr_array[5];
    
    /* TYPE_POINTER to array */
    int_array* GTY(()) array_ptr;
    
    /* TYPE_USER_STRUCT member */
    struct user_struct user_data;
    
    /* TYPE_LANG_STRUCT member */
    struct lang_struct lang_data;
    
    /* TYPE_CALLBACK member */
    callback_fn handler;
    
    /* TYPE_STRING member */
    const char* GTY(()) name;
    
    /* Chain pointers for linked list */
    struct container* GTY((skip("0"))) prev;
    struct container* GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct container global_container;
extern GTY(()) struct opaque_struct* global_opaque_ptr;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
    int defined_now;
    struct container* GTY((skip)) cont;
};

/* Array of different types to test array processing */
typedef union GTY(()) {
    struct my_struct s;
    union my_union u;
    base_ptr p;
} variant;

extern GTY(()) variant variant_array[20];

/* Struct with length attribute for variable-sized array */
struct GTY(()) varray_struct {
    int count;
    int GTY((length("%0.count"))) data[];
};

/* Nested pointer chain for deep traversal */
struct GTY(()) deep_node {
    int value;
    struct deep_node* GTY((skip)) left;
    struct deep_node* GTY((skip)) right;
    struct deep_node* GTY((chain_next("%0.next"))) next;
};

/* Root of deep structure */
extern GTY(()) struct deep_node* deep_root;
