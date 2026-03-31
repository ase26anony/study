#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip)) *
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* GTY structure with all delimiter types */
struct GTY((desc("%0.tag"), param_is(struct variant))) node {
    int value;
    int tag;
    
    /* Parentheses: function pointer with complex signature */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: nested arrays with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: nested union with GTY annotation */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((skip)) link;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Function pointer using macro */
    FUNC_PTR(int, (struct node *, int)) traverse_fn;
};

/* Recursive structure definition */
struct GTY(()) tree {
    struct node * GTY((skip)) root;
    int (* GTY((skip)) compare)(struct node *a, struct node *b);
    
    /* Nested structure with braces */
    struct {
        int count;
        struct node ** GTY((length("count"))) nodes;
    } cache;
};

/* Template-like structure with conditional attributes */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) list_node {
    int data;
    struct list_node * GTY((skip)) next;
    struct list_node * GTY((skip)) prev;
    
    /* Union with function pointers */
    union {
        int (* GTY((skip)) int_func)(int);
        void (* GTY((skip)) void_func)(void);
    } processor;
};

/* Global GTY variable */
extern struct tree * GTY(()) global_tree;

/* Typedef with GTY */
typedef struct node * GTY((skip)) node_ptr;

#endif /* TEST_GTY_H */
