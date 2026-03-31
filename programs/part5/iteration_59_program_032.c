#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] depth

/* Macro for function pointer with GTY skip attribute */
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Complex recursive structure with all delimiter types */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    int child_count;
    
    /* Parentheses: Function pointer with explicit argument list */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Using macro with nested brackets */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array with GTY annotation */
    struct node ** GTY((length("child_count"))) child_ptrs[];
    
    struct node *next;
    struct node *prev;
};

/* Union type with GTY conditional attributes */
union GTY((desc("%0.tag"))) variant_data {
    int GTY((tag("0"))) int_val;
    double GTY((tag("1"))) double_val;
    char * GTY((tag("2"))) string_val;
    struct node * GTY((tag("3"))) node_ptr;
};

/* Structure with deeply nested GTY attributes */
struct GTY(()) complex_struct {
    /* Function pointer with complex signature (parentheses) */
    void (* GTY((skip)) 
           complex_callback)(struct node *n, 
                             union variant_data * GTY((skip)) data,
                             int (* GTY((skip)) filter)(int));
    
    /* Array of function pointers (brackets + parentheses) */
    int (* GTY((skip)) callbacks[5])(struct node *, int);
    
    /* Nested structure with GTY (braces) */
    struct {
        int id;
        char * GTY((length("strlen(name)+1"))) name;
        struct node * GTY((skip)) nodes[10];
    } GTY(()) metadata;
    
    /* Multi-dimensional array with GTY */
    union variant_data * GTY((length("dim1"), 
                              param_is(union variant_data))) matrix[][10];
};

/* Typedef with GTY annotation containing all delimiter types */
typedef struct GTY((for_user)) tree_node {
    int value;
    
    /* Complex GTY attribute with string literal containing parentheses */
    char * GTY((desc("(%0.value == 0) ? \"zero\" : \"non-zero\""))) desc;
    
    /* Array of pointers to functions returning pointers (nested) */
    struct node * (* GTY((skip)) processors[3])(struct node *, int);
    
    /* Nested anonymous struct with GTY */
    struct {
        int depth;
        struct tree_node * GTY((chain_next("next_child"))) first_child;
        struct tree_node *last_child;
    } GTY(()) children_info;
    
    /* Union with GTY conditional based on tag */
    union {
        int as_int;
        double as_double;
        struct node * GTY((tag("2"))) as_node;
    } GTY((desc("%0.value > 0 ? 0 : 1"))) data;
    
} tree_node_t;

/* Global variable with GTY annotation */
extern tree_node_t * GTY((root)) global_tree_root;

/* Function pointer type with GTY skip */
typedef void (* GTY((skip)) traversal_func)(tree_node_t *node, void *user_data);

#endif /* TEST_GTY_H */
