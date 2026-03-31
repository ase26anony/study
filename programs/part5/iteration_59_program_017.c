#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Template-like macro with nested brackets */
#define PTR_ARRAY(type, len_field) type * GTY((length(#len_field))) []
#define VARRAY(type) type GTY((length("0"))) []

/* Complex attribute macro with nested parentheses */
#define CALLBACK_ATTR GTY((skip, callback("traverse_callback")))

/* Union tag macro with string literal */
#define TAG_ATTR(tag_val) GTY((tag(#tag_val)))

/* Primary recursive structure with all delimiter types */
struct GTY((desc("%0.tag"), chain_next("%0.next"), chain_prev("%0.prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* CALLBACK_ATTR traverse)(struct node * GTY((skip)) self, 
                                   int (* GTY((skip)) visitor)(void *, int), 
                                   void * GTY((skip)) context);
    
    /* Brackets: Nested array with variable bounds */
    struct node * GTY((length("child_count"), reorder("reorder_children"))) *children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * TAG_ATTR(0) data;
        struct node * TAG_ATTR(1) link;
    } GTY((desc("%0.tag"))) variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node, grandchild_count) grandchildren;
    
    /* Another level: array of function pointers */
    int (** GTY((length("callback_count"), skip)) callbacks)(int, char *[]);
    
    struct node *next;
    struct node *prev;
    unsigned int child_count;
    unsigned int grandchild_count;
    unsigned int callback_count;
};

/* Union type with nested GTY attributes */
union GTY((desc("%1.type"))) complex_union {
    struct node * GTY((tag("NODE_TYPE"))) as_node;
    int * GTY((tag("INT_ARRAY_TYPE"))) int_array;
    void (* GTY((tag("FUNC_TYPE"), skip)) func)(struct node *[], int);
};

/* Typedef with GTY and nested attributes */
typedef struct GTY(()) tree {
    struct node * GTY((reorder("sort_nodes"))) root;
    union complex_union GTY((desc("%0.type"))) data;
    
    /* Array with nested structure type */
    struct {
        int depth;
        struct node * GTY((skip)) path[10];
    } GTY((length("max_depth"))) traversal_info[];
    
    int max_depth;
    enum { INORDER, PREORDER, POSTORDER } traversal_type;
} tree_t;

/* Global variable declarations */
extern tree_t * GTY(()) global_tree;
extern struct node ** GTY((length("global_node_count"))) global_nodes;

#endif /* GTY_TEST_H */
