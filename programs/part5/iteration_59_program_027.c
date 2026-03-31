#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] GTY((depth))

/* Macro for function pointers with GTY attributes */
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Primary recursive tree structure with deeply nested GTY annotations */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with GTY attribute */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Using macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array with nested brackets */
    struct node ** GTY((length("grandchild_count"))) grandchild_ptrs[];
    
    /* Union with function pointer array */
    union {
        int (* GTY((skip)) func_array[5])(int, char *);
        struct {
            char * GTY((string)) name;
            int (* GTY((skip)) methods[3])();
        } obj;
    } GTY((desc("%0.variant.tag"))) extra;
    
    struct node *next;
    struct node *prev;
};

/* Structure with template-like macro expansion */
struct GTY(()) tree_container {
    /* Multi-dimensional array with GTY attributes */
    struct node * GTY((length("dim1"), nested_ptr)) matrix[][10];
    
    /* Union containing arrays of function pointers */
    union {
        void (* GTY((skip)) void_funcs[5])();
        int (* GTY((skip)) int_funcs[3])(int, float);
    } GTY((desc("1"))) func_union;
    
    /* Structure with nested GTY in typedef */
    struct {
        int count;
        struct node ** GTY((length("count"))) nodes;
    } GTY((param_is(struct node_list))) node_list;
};

/* Typedef with complex GTY attributes */
typedef struct GTY((for_user)) graph_edge {
    struct node * GTY((skip)) from;
    struct node * GTY((skip)) to;
    int weight;
    
    /* Array of function pointers in typedef */
    void (* GTY((skip)) handlers[3])(struct graph_edge *);
    
    /* Nested structure with union */
    struct {
        union {
            char * GTY((string)) label;
            int * GTY((length("label_len"))) label_data;
        } GTY((desc("%0.type"))) label_info;
        int type;
    } edge_data;
} edge_t;

/* Global variable declarations with GTY */
extern struct node * GTY((root)) global_tree_root;
extern edge_t ** GTY((length("edge_count"))) global_edges;

/* Function pointer type with GTY attribute */
typedef int (* GTY((skip)) node_visitor)(struct node *n, void * GTY((skip)) context);

#endif /* TEST_GTY_H */
