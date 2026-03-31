#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] depth
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback) args

/* Conditional GTY attributes with string literals */
#define VARIANT_ATTRS GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_ATTRS(count) GTY((length(#count)))

/* Primary recursive structure with all delimiter types */
struct GTY((for_user)) node {
    int value;
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Parentheses: Function pointer with explicit args */
    int (* GTY((skip)) traverse)(struct node *self, int (* GTY((skip)) visitor)(void *));
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next_node;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Complex callback with nested parentheses */
    CALLBACK_TYPE(int, (struct node *child, int depth, void * GTY((skip)) context));
};

/* Union with multiple GTY annotations */
union GTY((desc("%1.tag"))) tree_node {
    struct {
        int GTY((skip)) type;
        union tree_node * GTY((tag("0"))) left;
        union tree_node * GTY((tag("1"))) right;
    } binary;
    struct {
        int GTY((skip)) type;
        union tree_node * GTY((length("count"))) children[];
    } nary;
};

/* Template-like structure with deeply nested attributes */
struct GTY(()) container {
    /* Nested array of pointers with multiple bracket pairs */
    struct node * GTY((length("outer_len"))) matrix[][];
    
    /* Function pointer array with complex signature */
    void (* GTY((skip)) handlers[5])(struct container * GTY((skip)), 
                                     int (* GTY((skip)))(void *, void *));
    
    /* Union within struct within GTY */
    union {
        struct {
            int (* GTY((skip)) compare)(const void *, const void *);
            void * GTY((tag("0"))) data;
        } GTY((tag("0"))) sorted;
        struct {
            struct node ** GTY((length("size"))) items;
            int size;
        } GTY((tag("1"))) unsorted;
    } storage;
};

/* Global GTY variables */
extern struct node * GTY(()) root_node;
extern union tree_node * GTY(()) tree_root;

#endif /* TEST_GTY_H */
