#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] GTY((depth))

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))
#define CALLBACK_ATTR GTY((skip, callback("traverse_callback")))

/* Complex recursive structure with all delimiter types */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with explicit argument list */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Using macro expansion for nested array */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Multiple levels of nesting */
    struct node * GTY((nested_ptr)) next;
    struct node * GTY((nested_ptr)) prev;
    
    /* Array of function pointers (parentheses within brackets) */
    int (** GTY((length("callback_count"))) callbacks)(struct node *, int);
    
    int child_count;
    int callback_count;
    size_t len;
};

/* Union with GTY markers containing all delimiter types */
union GTY((desc("%0.type"))) complex_union {
    int type;
    
    /* Nested structure with array */
    struct {
        int GTY((tag("1"))) count;
        struct node * GTY((length("count"))) nodes[];
    } GTY((tag("1"))) node_list;
    
    /* Function pointer with complex signature */
    struct node * (* GTY((tag("2"))) 
                  creator)(int (*)(const char *), 
                          struct node * GTY((length("arg_count"))) []);
    
    /* Multi-dimensional array */
    int GTY((tag("3"))) matrix[3][4];
};

/* Template-like structure using macros */
struct GTY(()) tree_container {
    /* Macro expands to: struct node * GTY((length("len"))) [] */
    PTR_ARRAY(struct node) primary_nodes;
    
    /* Double macro expansion */
    NESTED_PTR_ARRAY(struct node, 2) deep_nodes;
    
    /* Union with GTY attribute containing quoted string with parentheses */
    union {
        char * GTY((tag("0"))) name;
        struct tree_container * GTY((tag("1"),
                                    desc("container->%0.name"))) 
                                    subcontainer;
    } GTY((desc("%0.union_tag"))) data;
    
    size_t len;
    int union_tag;
};

/* Global variable declarations with GTY markers */
extern struct node * GTY(()) global_node_list;
extern union complex_union GTY(()) global_union;

/* Typedef with GTY marker */
typedef struct node * GTY(()) node_ptr_t;

#endif /* TEST_GTY_H */
