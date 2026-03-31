/* Test header for covering gengtype-state.cc switch cases */

/* TYPE_UNDEFINED: Forward declaration creates undefined type */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type */
extern GTY(()) int global_scalar;

/* TYPE_STRING: String type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: Function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_ARRAY: Fixed-size array type */
typedef int GTY(()) int_array[10];
extern GTY(()) int_array global_array;

/* TYPE_POINTER: Pointer type */
struct GTY(()) base_struct {
    int id;
    struct base_struct* GTY((skip)) next;
};
typedef struct base_struct* GTY(()) base_ptr;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
    int field1;
    double field2;
    base_ptr GTY((tag("0"))) ptr_field;
    int_array array_field;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
    void* GTY((skip)) data;
    int data_size;
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int i;
    void* p;
    struct my_struct* s;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_type {
    TEST_NODE_TYPE1,
    TEST_NODE_TYPE2
};

struct GTY((desc("TEST_NODE"))) lang_struct {
    enum test_node_type code;
    union GTY((desc("%1.code"))) {
        struct GTY((tag("TEST_NODE_TYPE1"))) {
            int value;
            struct lang_struct* GTY((tag("0"))) child;
        } type1;
        struct GTY((tag("TEST_NODE_TYPE2"))) {
            double value;
            struct lang_struct* GTY((tag("1"))) left;
            struct lang_struct* GTY((tag("1"))) right;
        } type2;
    } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) complex_nested {
    /* Contains all different types */
    struct my_struct GTY((skip)) embedded_struct;
    union my_union GTY((skip)) embedded_union;
    struct user_struct* GTY((chain_next("next"), chain_prev("prev"))) user_list;
    struct lang_struct* GTY((tag("0"))) lang_node;
    callback_fn GTY((skip)) handlers[5];
    const char* GTY((length("strlen(%h.field) + 1"))) dynamic_string;
    int GTY((skip)) count;
    struct complex_nested* GTY((skip)) recursive_ptr;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct complex_nested global_complex;
extern GTY(()) struct opaque_struct* global_opaque_ptr;

/* Now define the previously opaque struct */
struct GTY(()) opaque_struct {
    int defined_now;
    struct my_struct* GTY((skip)) link;
};

/* Array of pointers with length field */
struct GTY(()) ptr_array_container {
    int count;
    struct my_struct* GTY((length("%h.count"))) items;
};

/* Union with desc/tag for callback testing */
union GTY((desc("%0.type"))) callback_union {
    int type;
    struct GTY((tag("1"))) {
        callback_fn func;
        void* GTY((skip)) context;
    } callback;
    struct GTY((tag("2"))) {
        int value;
    } data;
};
