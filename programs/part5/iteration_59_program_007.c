/* Complex GTY type definitions to exercise consume_balanced() logic */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrapper that expands to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested)) * GTY((length("outer_len"))) []

/* Another macro that creates function pointer with GTY skip */
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip))) args

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Complex recursive tree node structure */
typedef struct node node_t;

struct node GTY(()) {
    int value;
    
    /* Parentheses: Function pointer with GTY skip attribute */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next_node;
    } variant;
    
    /* Using macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array with nested GTY attributes */
    struct node * GTY((length("level2_count"))) * GTY((length("level1_count"))) level_array[];
    
    /* Function pointer with complex signature (multiple parentheses) */
    void (* GTY((skip)) complex_handler)(int (*filter)(const char *str), void **data);
};

/* Union type with GTY conditional attributes */
union tree_union GTY((param_is(struct node))) {
    struct node * GTY((tag("NODE_TYPE"))) as_node;
    int * GTY((tag("INT_TYPE"))) as_int_ptr;
    void (* GTY((tag("FUNC_TYPE"), skip)) as_func)(void);
};

/* Structure with deeply nested GTY annotations */
struct container GTY(()) {
    /* Array of pointers to arrays (multiple bracket pairs) */
    struct node * GTY((length("container_size"))) * GTY((length("array_size"))) node_matrix[];
    
    /* Union with GTY attributes containing string literals */
    union {
        char * GTY((desc("%0.str_field"))) str_data;
        struct node * GTY((desc("%0.node_field"), param_is(struct node))) node_data;
    } GTY((desc("%0.tag"))) data_union;
    
    /* Function pointer array */
    int (* GTY((length("func_count"), skip)) handlers[])(struct node *n, int flags);
    
    /* Nested structure with its own GTY markers */
    struct {
        int depth;
        struct node * GTY((chain_next("%0.next"), chain_prev("%0.prev"))) chain_node;
    } GTY((skip)) metadata;
};

/* Template-like structure using multiple macro expansions */
struct complex_tree GTY(()) {
    /* Using NESTED_PTR_ARRAY macro which expands to complex GTY annotation */
    NESTED_PTR_ARRAY(struct node) nested_nodes;
    
    /* Callback with complex signature using macro */
    CALLBACK_TYPE(void, (struct container *c, int (*validator)(const char *)));
    
    /* GTY attribute with escaped string literal */
    char * GTY((desc("Escaped string: %\"quoted%\" %0.name"))) name;
};

/* Global variable declarations with GTY markers */
extern struct node * GTY(()) global_tree_root;
extern struct container GTY(()) global_container;

/* Function pointer type with GTY skip and nested parentheses */
typedef void (* GTY((skip)) traversal_func_t)(
    struct node *root,
    void (*visit)(struct node *, void *),
    void *user_data
);

#endif /* TEST_GTY_H */
