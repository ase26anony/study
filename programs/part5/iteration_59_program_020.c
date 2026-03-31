#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested)) * GTY((length("outer_len"))) []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_DESC GTY((desc("items[%0.count]"), param_is(struct item_array)))

/* Complex recursive structure with all delimiter types */
struct GTY((chain_next ("next"), chain_prev ("prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) callback)(
        struct node *child, 
        int depth,
        void (* GTY((skip)) nested_cb)(int, char **)
    );
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            char * GTY((string)) name;
            int (* GTY((skip)) methods[3])(void);
        } GTY((desc("named_obj"))) obj;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array with nested GTY */
    struct node * GTY((length("level2_count"))) * GTY((length("level1_count"))) ancestors[][];
    
    struct node *next;
    struct node *prev;
};

/* Union with tag and param_is attributes */
union GTY((desc("%0.kind"))) tree_node {
    struct {
        int kind;
        union tree_node * GTY((tag("0"))) left;
        union tree_node * GTY((tag("1"))) right;
    } GTY((desc("binary"))) binary;
    
    struct {
        int kind;
        char * GTY((string)) value;
        int (* GTY((skip)) validators[2])(
            const char *str,
            int (* GTY((skip)) check_fn)(int)
        );
    } GTY((desc("leaf"))) leaf;
};

/* Template-like structure with deeply nested GTY */
struct GTY(()) container {
    /* Multiple levels of parentheses in attributes */
    struct node * GTY((reorder("sort_nodes"), 
                      param_is(struct node *),
                      user("my_module"))) root;
    
    /* Array of function pointers with GTY skip */
    void (* GTY((skip)) handlers[4])(
        struct container *ctr,
        int mode,
        void * GTY((user)) user_data
    );
    
    /* Nested structure with its own GTY */
    struct {
        int count;
        union tree_node ** GTY((length("count"))) items;
        int (* GTY((skip)) compare)(
            const union tree_node *a,
            const union tree_node *b,
            int options
        );
    } GTY((desc("storage"))) storage;
};

/* Global GTY variables */
extern struct container * GTY(()) global_container;
extern union tree_node * GTY((length("global_tree_count"))) global_trees[];

#endif /* TEST_GTY_H */
