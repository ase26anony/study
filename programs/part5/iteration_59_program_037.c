#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Requirement 3: Template-like macro expansions */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) * GTY((length("depth"))) []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip))) args

/* Requirement 4: Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant_node)))
#define UNION_TAG(tag_val) GTY((tag(#tag_val)))

/* Primary recursive structure with deeply nested annotations */
struct GTY(()) node {
    int value;
    
    /* Requirement 2: Brackets for array bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Requirement 3: Macro expansion with nested arrays */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Requirement 2: Parentheses for function pointers */
    int (* GTY((skip)) traverse_callback)(struct node *current, int depth);
    
    /* Requirement 2: Braces for nested union */
    union {
        int tag;
        void * GTY((tag("0"))) data_ptr;
        struct node * GTY((tag("1"))) node_ptr;
    } GTY((desc("%0.tag"))) variant;
    
    /* Complex nested function pointer type */
    CALLBACK_TYPE(void, (struct node **nodes, int count, 
                         int (* GTY((skip)) filter)(struct node *))) processor;
};

/* Requirement 6: GTY typedef */
typedef struct node GTY(()) node_t;

/* Another structure with different nesting patterns */
struct GTY(()) tree_container {
    /* Nested structure with GTY marker */
    struct GTY(()) metadata {
        int version;
        char * GTY((length("strlen(name)+1"))) name;
        struct metadata * GTY((skip)) next;
    } *meta;
    
    /* Array of pointers to arrays */
    node_t * GTY((length("tree_count"))) * GTY((length("max_depth"))) trees[];
    
    /* Union with tag */
    union GTY((desc("%0.type"))) {
        int type;
        struct {
            int (* GTY((skip)) compare)(node_t *a, node_t *b);
            void (* GTY((skip)) visit)(node_t *n, 
                void (* GTY((skip)) callback)(int, const char *));
        } GTY((tag("1"))) ops;
        struct {
            node_t ** GTY((length("size"))) nodes;
            int size;
        } GTY((tag("2"))) array;
    } container_type;
};

/* Requirement 6: Global variable with GTY */
extern node_t * GTY(()) global_tree_root;

/* Function pointer type with complex signature */
typedef int (* GTY((skip)) complex_callback_t)(
    struct tree_container * GTY((skip)) container,
    node_t * GTY((skip)) * GTY((length("count"))) nodes,
    int count,
    void (* GTY((skip)) logger)(const char *fmt, ...)
);

#endif /* TEST_GTY_H */
