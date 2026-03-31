#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Template-like macro with nested brackets */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("outer_len"))) * GTY((length("inner_len"))) []

/* Conditional attribute macro */
#define VARIANT_ATTRS GTY((desc("%0.tag"), param_is(struct variant), chain_next("%0.next")))

/* Complex GTY structure with all delimiter types */
typedef struct node node_t;

struct GTY(()) node {
    int value;
    
    /* Parentheses: function pointer with GTY attribute */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: nested array with variable bounds */
    struct node * GTY((length("child_count"))) * GTY((length("2"))) children[2][];
    
    /* Braces: nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Using template-like macro expansion */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Multiple levels of GTY attributes with string literals */
    struct node * GTY((chain_next("%0.next"), chain_prev("%0.prev"))) next;
    struct node * prev;
    
    /* Function pointer with complex signature (parentheses) */
    void (* GTY((skip)) complex_handler)(
        struct node * GTY((skip)) nodes[],
        int (* GTY((skip)) filter)(int, void *),
        void * GTY((skip)) context
    );
};

/* Union type with GTY markers */
union GTY((desc("%0.type"))) tagged_union {
    int type;
    struct {
        int count;
        node_t * GTY((length("count"))) items[];
    } GTY((tag("1"))) array;
    struct {
        char * GTY((skip)) name;
        double values[3];
    } GTY((tag("2"))) record;
};

/* Global variable with GTY marker */
extern node_t * GTY(()) global_tree_root;

/* Function pointer type with GTY */
typedef void (* GTY((skip)) traversal_func_t)(
    node_t *node,
    int level,
    void * GTY((skip)) data
);

#endif /* TEST_GTY_H */
