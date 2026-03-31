#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip, desc("%0"))) *
#define FUNC_PTR(ret, args) ret (* GTY((skip))) args

/* Conditional attribute macro */
#define VARIANT_ATTR(tagval) GTY((desc("%0.tag == " #tagval), param_is(struct variant)))

/* Base GTY structure with all delimiter types */
struct GTY((desc("%0.value"), chain_next("%0.next"), chain_prev("%0.prev"))) base_node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) callback)(struct base_node *self, 
                                 struct base_node * GTY((skip)) children[], 
                                 int depth);
    
    /* Brackets: Nested array with variable length */
    struct base_node * GTY((length("%0.child_count"))) children[];
    
    /* Braces: Embedded union */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct base_node * GTY((tag("1"))) node_ref;
    } variant;
    
    struct base_node *next;
    struct base_node *prev;
};

/* Recursive tree structure using macros */
struct GTY(()) tree_node {
    int value;
    int child_count;
    
    /* Using macro with nested brackets */
    PTR_ARRAY(struct tree_node) children;
    
    /* Nested function pointer with parentheses */
    FUNC_PTR(int, (struct tree_node *node, int (* GTY((skip)) helper)(int)));
    
    /* Union with GTY attributes in braces */
    union GTY((desc("%0.type"))) {
        int int_val;
        double GTY((tag("1"))) dbl_val;
        struct tree_node * GTY((tag("2"))) child_node;
        char * GTY((tag("3"), length("%0.str_len"))) string;
    } data;
    
    /* Complex attribute with string literal containing special chars */
    char * GTY((desc("name: \"%0.name\""))) name;
};

/* Template-like structure with multiple GTY annotations */
typedef struct GTY((for_user)) container {
    /* Array of pointers to arrays */
    struct tree_node * GTY((length("%0.outer_len"))) * GTY((length("%0.inner_len"))) matrix[];
    
    /* Function pointer array */
    void (* GTY((length("%0.func_count"), skip)) handlers[])(struct container *);
    
    /* Nested structure in union */
    union {
        struct {
            int count;
            struct tree_node * GTY((length("%0.count"))) items[];
        } GTY((tag("0"))) list;
        struct {
            char *key;
            struct tree_node *value;
        } GTY((tag("1"))) pair;
    } data;
} container_t;

/* External GTY declaration */
extern struct tree_node * GTY(()) global_tree;

#endif /* TEST_GTY_H */
