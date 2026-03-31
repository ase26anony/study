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

/* TYPE_ARRAY - fixed-size array type */
typedef int GTY(()) int_array[10];
extern GTY(()) int_array global_array;

/* TYPE_POINTER - pointer type */
struct GTY(()) base_struct {
    int id;
    struct base_struct* GTY((skip)) next;
};
typedef struct base_struct* GTY(()) base_ptr;
extern GTY(()) base_ptr global_pointer;

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
    int size;
};

/* TYPE_UNION - union type */
union GTY(()) my_union {
    int i;
    void* p;
    double d;
    struct my_struct* GTY((tag("1"))) s;
};

/* TYPE_LANG_STRUCT - language-specific structure */
enum test_node_codes {
    TEST_NODE_ARRAY,
    TEST_NODE_STRUCT,
    TEST_NODE_UNION
};

struct GTY((desc("TEST_NODE"))) lang_struct {
    enum test_node_codes code;
    union GTY((desc("%1.code"))) {
        struct my_struct GTY((tag("TEST_NODE_STRUCT"))) s;
        union my_union GTY((tag("TEST_NODE_UNION"))) u;
        int_array GTY((tag("TEST_NODE_ARRAY"))) a;
    } GTY((tag("0"))) u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container {
    struct my_struct nested_struct;
    union my_union nested_union;
    struct lang_struct* GTY((tag("1"))) lang_ptr;
    callback_fn callback_field;
    struct user_struct* GTY((chain_next("next"), chain_prev("prev"))) user_chain;
    struct container* GTY((skip)) next_container;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_struct_var;
extern GTY(()) union my_union global_union_var;
extern GTY(()) struct lang_struct global_lang_struct_var;
extern GTY(()) struct container global_container;
extern GTY(()) struct user_struct global_user_struct;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
    int defined_now;
    struct container* GTY((skip)) cont_ptr;
};

/* Array of pointers for additional coverage */
typedef struct container* GTY(()) container_ptr_array[5];
extern GTY(()) container_ptr_array global_ptr_array;

/* Struct with length field for array */
struct GTY(()) variable_array_container {
    int count;
    struct my_struct GTY((length("%h.count"))) items[1];
};
