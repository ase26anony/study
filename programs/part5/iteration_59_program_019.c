#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip)) *
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Complex type definition with all delimiter types */
typedef struct node node_t;

struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) callback)(struct node *child, int depth, 
                                 void (* GTY((skip)) helper)(int));
    
    /* Brackets: Nested arrays with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            char * GTY((length("str_len"))) name;
            int (* GTY((skip)) methods[3])(void);
        } GTY((desc("tag == 1"))) obj;
    } variant;
    
    /* Macro-expanded type with nested brackets */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer with skip attribute */
    NESTED_PTR(struct node) parent_ref;
    
    struct node *next;
    struct node *prev;
};

/* Global GTY variable declaration */
extern node_t * GTY(()) global_tree_root;

/* Function pointer type with GTY attributes */
typedef FUNC_PTR(int, (node_t *, int, void *)) traversal_fn_t;

/* Union type with conditional attributes */
union GTY((desc("%0.tag"))) variant {
    int tag;
    struct {
        int count;
        node_t ** GTY((length("count"))) items;
    } GTY((tag("1"))) list;
    struct {
        char *key;
        void *value;
    } GTY((tag("2"))) pair;
};

#endif /* TEST_GTY_H */
