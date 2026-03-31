/* Test header to cover all gengtype-state.cc switch cases */
#ifndef GTYPE_COVERAGE_TEST_H
#define GTYPE_COVERAGE_TEST_H

/* Forward declaration for TYPE_UNDEFINED case */
struct GTY(()) opaque_struct;

/* TYPE_SCALAR: Fundamental scalar type as GC root */
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
    const char* GTY(()) name;
};

typedef struct base_struct* GTY(()) struct_ptr;
extern GTY(()) struct_ptr global_ptr;

/* TYPE_STRUCT: Plain C struct */
struct GTY(()) my_struct {
    int field1;
    double field2;
    struct base_struct* GTY(()) nested_ptr;
    int_array array_field;
};

/* TYPE_USER_STRUCT: Struct with user-defined marking */
struct GTY((user)) user_struct {
    void* data;
    int length;
    /* User-defined marking function would be declared elsewhere */
};

/* TYPE_UNION: Union type */
union GTY(()) my_union {
    int i;
    double d;
    void* GTY(()) p;
    struct my_struct* GTY(()) s;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
enum test_node_codes {
    TEST_NODE_BASE,
    TEST_NODE_DERIVED
};

struct GTY((desc("TEST_NODE"))) lang_struct {
    int code;
    union GTY((desc("1"))) {
        struct base_struct* GTY((tag("0"))) base;
        struct my_struct* GTY((tag("1"))) derived;
    } u;
};

/* Complex nested structure to ensure deep traversal */
struct GTY(()) container_struct {
    /* Chain of structures */
    struct container_struct* GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
    struct container_struct* GTY((skip)) prev;
    
    /* Array of pointers */
    struct base_struct* GTY(()) ptr_array[5];
    
    /* Union field */
    union my_union data;
    
    /* String with length */
    const char* GTY(()) text;
    int GTY((length("%h.text ? strlen(%h.text) : 0"))) text_length;
    
    /* Callback */
    callback_fn handler;
    
    /* Nested lang struct */
    struct lang_struct lang_node;
};

/* Variable declarations to ensure inclusion in GC roots */
extern GTY(()) struct my_struct global_my_struct;
extern GTY(()) union my_union global_my_union;
extern GTY(()) struct user_struct global_user_struct;
extern GTY(()) struct container_struct global_container;
extern GTY(()) struct lang_struct global_lang_struct;

/* Now define the previously opaque struct for TYPE_UNDEFINED resolution */
struct GTY(()) opaque_struct {
    int defined_now;
    struct my_struct* GTY(()) link;
};

/* Array of unions */
typedef union my_union GTY(()) union_array[3];
extern GTY(()) union_array global_union_array;

/* Struct with skip option */
struct GTY((skip)) skipped_struct {
    int ignored_field;
    struct base_struct* GTY(()) important_ptr;
};

/* Complete the forward declaration chain */
extern GTY(()) struct opaque_struct global_opaque;

#endif /* GTYPE_COVERAGE_TEST_H */
