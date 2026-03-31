#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip)) *
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* GTY marker for recursive structures */
#define GTY_RECURSIVE GTY((chain_next("next"), chain_prev("prev")))

/* Complex type definitions */
typedef struct node node_t;

/* Function pointer type with GTY annotation */
typedef int (* GTY((skip)) node_callback)(node_t *child, int depth);

/* Union with GTY-tagged variant */
union variant_data {
    int tag;
    void * GTY((tag("0"))) data;
    char * GTY((tag("1"))) str;
    struct {
        int x;
        int y;
    } GTY((tag("2"))) point;
};

/* Primary recursive structure with all delimiter types */
struct node GTY((
    /* Parentheses in function pointer */
    desc("%0.callback ? \"has_callback\" : \"no_callback\""),
    param_is(node_t)
)) {
    int value;
    
    /* Function pointer with nested parentheses */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Array with brackets - nested array case */
    struct node * GTY((length("child_count"))) children[];
    
    /* Macro-expanded nested pointer array */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Union with braces */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) link;
        /* Nested structure in union */
        struct {
            char * GTY((length("str_len"))) name;
            int id;
        } GTY((tag("2"))) info;
    } variant;
    
    /* For linked list traversal */
    struct node * GTY_RECURSIVE next;
    struct node * GTY_RECURSIVE prev;
    
    /* Complex function pointer with parameter list */
    void (* GTY((skip)) complex_handler)(
        struct node *self,
        int (* GTY((skip)) filter)(struct node *, void *),
        void *user_data
    );
};

/* Global variable with GTY annotation */
extern struct node * GTY(()) global_tree_root;

/* Another structure with deeply nested GTY attributes */
struct container GTY(()) {
    /* Array of function pointers */
    node_callback GTY((length("callback_count"))) callbacks[];
    
    /* Nested structure with its own GTY */
    struct {
        struct node * GTY((skip)) *nodes;  /* Pointer to pointer */
        int count;
        /* Union inside nested structure */
        union {
            int mode;
            char * GTY((tag("1"))) config;
        } GTY((desc("%0.mode"))) settings;
    } GTY((skip)) manager;
    
    /* Variable length array of structures */
    struct node GTY((length("node_count"))) nodes[];
};

#endif /* TEST_GTY_H */
