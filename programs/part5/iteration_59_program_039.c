#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("count"), nested_ptr)) [] []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback) args

/* Conditional attribute macro */
#define VARIANT_ATTRS GTY((desc("%0.tag"), param_is(struct variant), chain_next("%0.next")))

/* Complex recursive structure with all delimiter types */
typedef struct node node_t;

struct GTY((for_user)) node {
    int value;
    
    /* Parentheses: function pointer with complex signature */
    int (* GTY((skip)) 
         traverse_func)(struct node * GTY((skip)) current, 
                       int depth, 
                       void * GTY((skip)) context);
    
    /* Brackets: nested arrays with variable bounds */
    struct node * GTY((length("child_count"), 
                       param_is(struct node))) children[];
    
    /* Braces: nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) link;
    } GTY((desc("%0.tag"))) variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Another function pointer with parentheses */
    CALLBACK_TYPE(void, (struct node *n, int action)) action_callback;
    
    /* Chain pointer for linked list */
    struct node * GTY((chain_next("%0.next"))) next;
};

/* Union type with GTY markers */
union GTY((desc("%1.tag"))) tree_union {
    int GTY((tag("0"))) int_val;
    double GTY((tag("1"))) double_val;
    struct node * GTY((tag("2"))) node_ptr;
    char * GTY((tag("3"))) string;
    
    /* Nested structure inside union */
    struct {
        int x;
        int y;
        void * GTY((skip)) metadata;
    } GTY((tag("4"))) point;
};

/* Template-like structure with multiple nested attributes */
struct GTY((
    for_user,
    desc("%0.type"),
    param_is(struct container)
)) container {
    int type;
    
    /* Array of arrays */
    int GTY((length("dim1"), nested)) matrix[][10];
    
    /* Pointer to array with callback */
    void (** GTY((length("callback_count"), skip)) 
          callbacks[][5])(struct container *c, int cmd);
    
    /* Union with nested GTY */
    union {
        struct node * GTY((tag("0"))) root;
        union tree_union * GTY((tag("1"))) utree;
        
        /* Anonymous struct inside union */
        struct {
            struct container * GTY((chain_prev("%p.prev"), 
                                   chain_next("%n.next"))) chain;
            int depth;
        } GTY((tag("2"))) nested;
    } GTY((desc("%0.u_tag"))) data;
};

/* Global variable declarations */
extern struct node * GTY(()) global_tree_root;
extern union tree_union GTY(()) global_union_array[];

#endif /* TEST_GTY_H */
