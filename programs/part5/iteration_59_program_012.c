/* test-gty.h - Complex GTY annotations to trigger consume_balanced calls */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested)) * GTY((skip)) []
#define FUNCTION_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals containing special chars */
#define DESC_ATTR(tag_field) desc("%0." #tag_field)
#define PARAM_IS(type) param_is(struct type)

/* Complex recursive structure with all delimiter types */
struct GTY(()) node {
    int value;
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Parentheses: Function pointer with explicit argument list */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((skip)) next;
    } GTY((desc("%0.tag"))) variant;
    
    /* Template-like macro expansion with nested brackets */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array with GTY attributes */
    struct node ** GTY((length("ptr_count"), 
                       reorder("sort_nodes"))) ptr_array;
    
    /* Function pointer returning function pointer */
    int (*(* GTY((skip)) complex_cb)(int))(struct node *, int);
};

/* Union type with GTY markers on members */
union GTY((desc("node_type"))) node_union {
    struct node * GTY((tag("NODE_TYPE_STRUCT"))) as_node;
    int * GTY((tag("NODE_TYPE_INT"))) as_int_array;
    void (* GTY((tag("NODE_TYPE_FUNC"))) as_func)(void);
};

/* Structure with deeply nested GTY attributes */
struct GTY(()) container {
    /* Array of pointers to arrays */
    struct node * GTY((length("outer_len"))) * GTY((length("inner_len"))) matrix[];
    
    /* Union containing structure with GTY */
    union {
        struct {
            struct node * GTY((chain_next("%h.next"))) next;
            struct node * GTY((chain_prev("%h.prev"))) prev;
        } GTY((skip)) links;
        int GTY((skip)) count;
    } GTY((desc("%0.type"))) header;
    
    /* Function pointer with complex signature */
    void (* GTY((callback)) processor)(
        struct container * GTY((skip)) self,
        int (* GTY((skip)) filter)(struct node *, void *),
        void * GTY((skip)) context
    );
    
    /* Nested structure definition inside GTY */
    struct {
        int depth;
        struct node * GTY((length("%h.depth"))) path[];
    } GTY((nested)) traversal_state;
};

/* Global variables with GTY markers */
extern struct node * GTY((root)) global_tree_root;
extern union node_union GTY((skip)) global_union_array[];

/* Typedef with GTY */
typedef struct node * GTY((ptr_alias("node_ptr"))) node_ptr_t;

#endif /* TEST_GTY_H */
