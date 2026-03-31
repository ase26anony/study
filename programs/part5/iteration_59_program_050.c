#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) * GTY((length("depth"))) []

/* Conditional GTY attributes with string literals containing special characters */
#define DESC_ATTR(tag_field) desc("%0." #tag_field)
#define PARAM_IS(type) param_is(struct type)

/* Complex recursive structure with multiple GTY annotation styles */
struct GTY((desc("tree_node"), chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses for function pointer with GTY skip attribute */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets for array bounds - nested arrays */
    struct node * GTY((length("child_count"))) children[];
    
    /* Using macro expansion for complex array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Braces for nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Multiple levels of pointer indirection with GTY attributes */
    struct node ** GTY((skip)) skip_ptr;
    
    /* Function pointer array with complex signature */
    void (* GTY((length("func_count"))) func_array[])(struct node *n, int (*)(int));
    
    struct node *next;
    struct node *prev;
};

/* Union type with GTY conditional attributes */
union GTY((desc("%0.tag"), param_is(struct variant_data))) variant_data {
    int GTY((tag("0"))) int_val;
    double GTY((tag("1"))) double_val;
    struct node * GTY((tag("2"))) node_ptr;
    char * GTY((tag("3"))) string;
};

/* Structure with deeply nested GTY attributes */
struct GTY(()) container {
    /* Array of pointers with length attribute containing parentheses */
    union variant_data * GTY((length("count + (extra ? 1 : 0)"))) data_array[];
    
    /* Function pointer with nested parentheses in type */
    int (* GTY((skip)) complex_func)(int (*callback)(int, int), struct node *);
    
    /* Nested structure with GTY marker */
    struct {
        int id;
        char * GTY((desc("%0.name"))) name;
        struct node * GTY((chain_next("next_in_group"))) group_members;
    } GTY((desc("sub_container"))) sub;
    
    /* Multi-dimensional array with GTY attributes */
    int * GTY((length("dim1 * dim2"))) matrix[];
};

/* Template-like macro for tree structures */
#define TREE_NODE(type) \
    struct GTY((desc("tree_node_" #type))) tree_node_##type { \
        type value; \
        struct tree_node_##type * GTY((length("child_count"))) children[]; \
        void (* GTY((skip)) processor)(struct tree_node_##type *, type); \
    }

/* Instantiate template-like macros */
TREE_NODE(int);
TREE_NODE(double);

/* Global variable declarations with GTY markers */
extern struct node * GTY((root)) global_tree_root;
extern union variant_data GTY((length("global_count"))) global_data_array[];

#endif /* TEST_GTY_H */
