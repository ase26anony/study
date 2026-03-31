#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len##depth"))) [] [depth]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Base GTY structure with all three delimiter types */
struct GTY((desc("%0.tag"), param_is(struct node))) node {
    int tag;
    char * GTY((length("strlen(name)+1"))) name;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) traverse)(struct node *self, 
                                 int (* GTY((skip)) callback)(struct node *, int),
                                 int depth);
    
    /* Brackets: Nested arrays with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Embedded union with GTY annotation */
    union {
        int int_val;
        char * GTY((tag("1"))) str_val;
        struct node * GTY((tag("2"))) node_val;
    } GTY((desc("%0.tag"))) variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
};

/* Recursive tree structure with multiple nesting levels */
struct GTY(()) tree {
    struct node * GTY((skip)) root;
    
    /* Function pointer array with parentheses */
    void (* GTY((length("callback_count"), skip)) callbacks[3])(
        struct tree * GTY((skip)),
        struct node * GTY((skip)),
        int GTY((skip))
    );
    
    /* Nested structure with braces */
    struct {
        int depth;
        struct node ** GTY((length("max_nodes"))) node_stack;
    } traversal_state;
    
    /* Union containing array of function pointers */
    union {
        int (* GTY((skip)) int_funcs[2])(int, int);
        struct node * (* GTY((skip)) node_funcs[2])(
            struct node * GTY((skip)), 
            struct node * GTY((skip))
        );
    } GTY((desc("%0.tag"))) func_union;
};

/* Template-like structure using macros */
struct GTY(()) complex_container {
    /* Multi-dimensional array with brackets */
    struct node * GTY((length("dim1"), skip)) matrix[][10];
    
    /* Macro with nested parentheses and brackets */
    NESTED_PTR_ARRAY(struct node, 3) deep_array;
    
    /* Complex callback signature */
    CALLBACK_TYPE(int, (struct node * GTY((skip)), 
                       int (* GTY((skip)))(int, int),
                       char * GTY((length("len"))) [])) processor;
};

/* Conditional GTY attributes with string literals */
struct GTY((desc("%0.type == 0 ? \"int\" : \"node\""),
           param_is(struct typed_value))) typed_value {
    int type;
    union {
        int int_val;
        struct node * GTY((tag("1"))) node_val;
        char * GTY((tag("2"), length("strlen(val)+1"))) str_val;
    } value;
};

/* Global GTY variables */
extern struct tree * GTY(()) global_tree;
extern struct node * GTY((length("global_count"))) global_nodes[];

/* Function pointer type with GTY skip */
typedef int (* GTY((skip)) node_visitor)(
    struct node * GTY((skip)),
    void * GTY((skip)) context,
    int (* GTY((skip)) should_skip)(struct node *)
);

#endif /* TEST_GTY_H */
