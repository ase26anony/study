#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback) args
#define NESTED_UNION(tag_val) union { \
    int tag; \
    void * GTY((tag(#tag_val))) data; \
}

/* GTY structure with deeply nested annotations */
struct GTY((desc("%0.tag"), param_is(struct node))) node {
    int value;
    int tag;
    
    /* Parentheses: function pointer with explicit argument list */
    CALLBACK_TYPE(int, (struct node *child, int depth));
    
    /* Brackets: nested array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: nested union inside structure */
    NESTED_UNION(0) variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Multiple levels of nested GTY attributes */
    struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) {
        struct node * GTY((skip)) next;
        struct node * GTY((skip)) prev;
    } links;
    
    /* Conditional attribute with string literal containing special chars */
    char * GTY((desc("%q[%0.tag]"), param_is(struct node))) name;
};

/* Typedef with GTY marker */
typedef struct GTY(()) tree_node {
    struct node * GTY((reorder("resort_tree"))) root;
    int (* GTY((skip)) traverse_fn)(struct node ** GTY((length("count"))) nodes[]);
    union {
        int flags[4];
        struct {
            int visited:1;
            int processed:1;
        } GTY((skip)) state;
    } metadata;
} tree_node_t;

/* Global variable with GTY marker */
extern tree_node_t * GTY(()) global_tree;

#endif /* TEST_GTY_H */
