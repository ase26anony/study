#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] GTY((depth))

/* Complex function pointer type with GTY attributes */
typedef int (* GTY((skip)) complex_callback_t)(
    void * GTY((tag("0"))) data,
    int depth,
    const char * GTY((length("strlen(%0)+1"))) message[]
);

/* Union with GTY-tagged variant */
union variant_data {
    int tag;
    void * GTY((tag("0"))) data;
    struct node * GTY((tag("1"))) node_ptr;
};

/* Primary recursive structure with deeply nested GTY annotations */
struct GTY((desc("%0.tag"), param_is(struct node))) node {
    int tag;
    char * GTY((length("strlen(%h->name)+1"))) name;
    
    /* Parentheses: Function pointer with complex signature */
    complex_callback_t GTY((callback("node_callback"))) callback;
    
    /* Brackets: Multiple array types with nested GTY attributes */
    struct node * GTY((length("child_count"))) children[];
    PTR_ARRAY(struct node) grandchildren;
    
    /* Braces: Nested union within structure */
    union {
        int int_value;
        double GTY((skip)) double_value;
        struct {
            char * GTY((string)) str_data;
            void * GTY((tag("%0.tag"))) ptr_data;
        } GTY((desc("1"))) nested;
    } variant;
    
    /* Template-like macro expansion with multiple levels */
    NESTED_PTR_ARRAY(struct node, 2) deep_children;
    
    /* Conditional GTY attributes with string literals containing special chars */
    union variant_data * GTY((desc("%0->tag"), 
                             param_is(union variant_data),
                             chain_next("%0->node_ptr"))) variant_ptr;
};

/* Global variable with GTY annotation */
extern struct node * GTY((root)) global_tree_root;

/* Function pointer typedef with GTY attributes in parameters */
typedef void (* GTY((skip)) traversal_func_t)(
    struct node * GTY((user("%0"), param_is(struct node))) current,
    int * GTY((user("%0"))) counter,
    const char * GTY((length("%2"))) path[]
);

/* Another structure with different delimiter combinations */
struct GTY(()) tree_container {
    /* Array of function pointers - nested parentheses */
    traversal_func_t GTY((length("func_count"))) handlers[];
    
    /* Multi-dimensional array with GTY attributes */
    struct node * GTY((length("dim1 * dim2"))) matrix[][];
    
    /* Union with GTY-tagged anonymous struct */
    union {
        struct {
            int x;
            int y;
        } GTY((skip)) point;
        struct node * GTY((chain_next("%h.next"))) node_chain;
    } GTY((desc("%0.tag"))) data;
    
    /* String literal in GTY attribute with escaped quotes */
    const char * GTY((desc("\"node_type: %0\""))) type_name;
};

#endif /* TEST_GTY_H */
