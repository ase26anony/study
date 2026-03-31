/* Test header to cover all gengtype-state.cc switch cases */

#ifndef TEST_COVERAGE_H
#define TEST_COVERAGE_H

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: fundamental scalar type */
extern GTY(()) int global_scalar;

/* TYPE_STRING: string type */
extern GTY(()) const char* global_string;

/* TYPE_CALLBACK: function pointer type */
typedef void (* GTY(()) callback_fn)(void);
extern GTY(()) callback_fn global_callback;

/* TYPE_ARRAY: fixed-size array type */
typedef int GTY(()) int_array[10];
extern GTY(()) int_array global_array;

/* TYPE_POINTER: pointer type */
struct GTY(()) base_struct {
    int id;
    struct base_struct* GTY((skip)) next;
};
typedef struct base_struct* GTY(()) base_ptr;
extern GTY(()) base_ptr global_base_ptr;

/* TYPE_STRUCT: plain C struct */
struct GTY(()) my_struct {
    int field1;
    double field2;
    base_ptr GTY((tag("0"))) ptr_field;
    int_array array_field;
};

/* TYPE_USER_STRUCT: struct with user option */
struct GTY((user)) user_struct {
    void* GTY((skip)) data;
    int data_size;
};

/* TYPE_UNION: union type */
union GTY(()) my_union {
    int i;
    void* p;
    struct my_struct* GTY((tag("1"))) s;
    double d;
};

/* TYPE_LANG_STRUCT: language-specific structure */
enum test_node_type {
    TEST_NODE_TYPE1,
    TEST_NODE_TYPE2,
    TEST_NODE_ARRAY
};

struct GTY((desc("test_node_type"))) lang_struct {
    enum test_node_type code;
    union GTY((desc("test_node_type"))) {
        struct GTY((tag("TEST_NODE_TYPE1"))) {
            int value;
            char* GTY((length("strlen($)"))) name;
        } type1;
        struct GTY((tag("TEST_NODE_TYPE2"))) {
            double x;
            double y;
        } type2;
        struct GTY((tag("TEST_NODE_ARRAY"))) {
            struct lang_struct** GTY((length("$->array_size"))) children;
            int array_size;
        } array_node;
    } GTY((tag("code"))) u;
};

/* Complex nesting to ensure deep traversal */
struct GTY(()) container_struct {
    /* Contains all different type kinds */
    struct my_struct nested_struct;          /* TYPE_STRUCT */
    union my_union nested_union;             /* TYPE_UNION */
    struct user_struct* GTY((skip)) user_ptr; /* TYPE_USER_STRUCT via pointer */
    base_ptr* GTY((length("ptr_count"))) ptr_array; /* TYPE_ARRAY of pointers */
    int ptr_count;
    callback_fn callbacks[5];                /* TYPE_ARRAY of callbacks */
    const char* GTY((length("name_len + 1"))) name; /* TYPE_STRING */
    int name_len;
    struct lang_struct* GTY((tag("0"))) lang_node; /* TYPE_LANG_STRUCT */
    struct opaque_struct* GTY((skip)) opaque; /* TYPE_UNDEFINED (forward declared) */
};

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
    int defined_now;
    struct container_struct* GTY((chain_next("next_opaque"))) next;
    struct opaque_struct* GTY((chain_prev("prev_opaque"))) prev;
};

/* Global variables to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct lang_struct global_lang_struct;
extern GTY(()) struct container_struct global_container;
extern GTY(()) struct opaque_struct global_opaque;

/* Chain of structures for chain_next/chain_prev testing */
struct GTY(()) chain_struct {
    int id;
    struct chain_struct* GTY((chain_next("next_in_chain"))) next;
    struct chain_struct* GTY((chain_prev("prev_in_chain"))) prev;
};

extern GTY(()) struct chain_struct* chain_head;

/* Array of structures */
typedef struct my_struct GTY(()) my_struct_array[5];
extern GTY(()) my_struct_array global_struct_array;

/* Nested array */
typedef struct container_struct* GTY(()) container_ptr_array[3][4];
extern GTY(()) container_ptr_array global_nested_array;

#endif /* TEST_COVERAGE_H */
