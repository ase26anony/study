#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] GTY((depth))

/* Macro for function pointer with GTY skip attribute */
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback)args

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Complex recursive structure with all delimiter types */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) traverse_callback)(struct node *self, 
                                          struct node * GTY((skip)) parent,
                                          int (* GTY((skip)) visitor)(int, void *));
    
    /* Brackets: Array with variable bounds (nested array case) */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            char * GTY((length("str_len"))) string;
            int (* GTY((skip)) compare)(const char *, const char *);
        } GTY((desc("1"))) str_info;
    } variant;
    
    /* Using macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer with nested GTY attributes */
    struct node ** GTY((skip, reorder("node_ptr_array_reorder"))) siblings;
    
    struct node *next;
    struct node *prev;
};

/* Union type with GTY conditional attributes */
union GTY((desc("%1.type"))) tree_node {
    struct node * GTY((tag("NODE_TYPE"))) n;
    void * GTY((tag("VOID_TYPE"))) v;
    int (* GTY((tag("FUNC_TYPE"), skip)) func_ptr)(int, 
                                                   struct node * GTY((skip)), 
                                                   union tree_node *);
};

/* Template-like structure with deeply nested GTY attributes */
struct GTY(()) container {
    /* Array of function pointers with GTY skip */
    int (** GTY((length("callback_count"), skip)) callbacks)(
        struct container *,
        int [3],  /* Nested array in parameter */
        void (*)(int, char **)  /* Function pointer parameter */
    );
    
    /* Nested structure with its own GTY markers */
    struct {
        int count;
        struct node * GTY((length("count"))) items[];
    } GTY((desc("nested"))) storage;
    
    /* Union with conditional tagging */
    union {
        int as_int;
        struct node * GTY((tag("AS_NODE"))) as_node;
        struct container * GTY((tag("AS_CONTAINER"))) as_container;
    } GTY((desc("%0.as_int"))) data;
};

/* Global variable declarations with GTY */
extern struct node * GTY((root)) global_tree_root;
extern union tree_node ** GTY((length("global_count"))) global_nodes;

/* Function pointer type with complex GTY attributes */
typedef int (* GTY((skip)) complex_callback_t)(
    struct node *,
    int (*)(struct node **, int),  /* Nested function pointer parameter */
    char * GTY((length("len"))) []  /* Array parameter with GTY */
);

#endif /* TEST_GTY_H */
