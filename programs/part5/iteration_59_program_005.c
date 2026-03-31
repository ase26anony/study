/* test-gty.h - Complex GTY annotations to exercise consume_balanced logic */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_ARRAY(type, dim) type GTY((length("size##dim"))) [] [dim]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))
#define FUNC_DESC GTY((desc("\"%0.name\" function"), chain_next("%0.next")))

/* Complex recursive structure with all delimiter types */
struct GTY((for_user)) node {
    int value;
    int child_count;
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) 
        traverse_callback)(struct node * GTY((skip)) current, 
                          int depth, 
                          void * GTY((skip)) user_data);
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) link;
    } GTY((desc("%0.tag"))) variant;
    
    /* Using macro with nested brackets */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Multi-dimensional array with GTY annotation */
    int * GTY((length("child_count * 2"))) matrix[];
};

/* Union with GTY markers containing all delimiter types */
union GTY((desc("union with nested delimiters"))) complex_union {
    struct {
        /* Nested parentheses in function pointer array */
        void (* GTY((skip)) handlers[4])(int, char * GTY((skip)));
        
        /* Array of arrays with GTY */
        char * GTY((length("str_count"))) strings[][32];
    } s;
    
    struct node * GTY((tag("NODE_TYPE"))) node_ptr;
    
    /* Union within union */
    union {
        int (* GTY((skip)) compare)(const void *, const void *);
        float (* GTY((skip)) transform[2])(float, float);
    } funcs;
};

/* Typedef with GTY and complex nested annotations */
typedef struct GTY(()) tree {
    enum { LEAF, BRANCH } type;
    
    /* Conditional attribute with string containing parentheses */
    union GTY((desc("%0.type == LEAF ? \"leaf\" : \"branch\""))) {
        struct {
            int value;
        } leaf;
        
        struct {
            struct tree * GTY((length("arity"))) children[];
            int arity;
            
            /* Function pointer with nested parentheses in arguments */
            int (* GTY((skip)) 
                validator)(struct tree *const * GTY((skip)) nodes, 
                          int count,
                          int (* GTY((skip)) check)(int));
        } branch;
    } u;
} tree_t;

/* Global variable with GTY annotation containing nested constructs */
extern tree_t * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) 
    global_tree_list;

/* Another structure with deeply nested GTY attributes */
struct GTY((for_user)) container {
    /* Array of function pointers with GTY skip */
    void (* GTY((skip)) callbacks[3])(
        struct container * GTY((skip)), 
        int (* GTY((skip)) filter)(char ** GTY((skip)))
    );
    
    /* Nested structure with GTY */
    struct {
        char * GTY((length("len + 1"))) buffer;
        int len;
        
        /* Union with tag */
        union GTY((tag("type"))) {
            int number;
            char * GTY((tag("1"))) text;
            struct node * GTY((tag("2"))) node;
        } data;
    } payload;
    
    /* Variable length array of complex type */
    struct {
        int id;
        tree_t * GTY((length("subtree_count"))) subtrees[];
    } * GTY((length("item_count"))) items;
};

#endif /* TEST_GTY_H */
