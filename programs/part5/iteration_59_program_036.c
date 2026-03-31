#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested)) * GTY((skip)) []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback) args

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))
#define COMPLEX_DESC GTY((desc("(%0.tag == 0) ? \"int\" : \"ptr\""), param_is(struct variant)))

/* Complex recursive structure with multiple GTY annotation styles */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses for function pointer with explicit argument list */
    int (* GTY((skip)) traverse_callback)(struct node *child, int depth);
    
    /* Brackets for array bounds - nested array declaration */
    struct node * GTY((length("child_count"))) children[];
    
    /* Using macro expansion for template-like array */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Braces for nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Function pointer with complex signature using CALLBACK_TYPE macro */
    CALLBACK_TYPE(void, (struct node *self, int (*)(int, char **))) complex_callback;
    
    struct node *next;
    struct node *prev;
};

/* Union with GTY conditional attributes */
union GTY((desc("%1.tag"))) tree_node {
    struct {
        int GTY((skip)) tag;
        union {
            int int_value;
            struct node * GTY((tag("1"))) node_ptr;
            char * GTY((tag("2"))) string;
        } GTY((desc("(%0.tag == 0) ? \"int\" : ((%0.tag == 1) ? \"node\" : \"string\")"))) data;
    } variant;
    
    /* Nested structure with array of function pointers */
    struct {
        int count;
        void (* GTY((length("count"))) handlers[3])(struct node *, int);
        struct node * GTY((length("count * 2"))) nodes[];
    } handler_list;
};

/* Typedef with GTY marker and nested attributes */
typedef struct GTY(()) graph_edge {
    struct node * GTY((refless("node"))) from;
    struct node * GTY((refless("node"))) to;
    
    /* Array with variable bounds specification */
    int * GTY((length("((%0.from != NULL) ? %0.from->value : 0)"))) weights;
    
    /* Union with tag and conditional param_is */
    union {
        int distance;
        float probability;
        struct node * GTY((tag("2"), param_is(struct node))) intermediate;
    } GTY((desc("%0.from->variant.tag"))) attr;
} edge_t;

/* Global variable declarations with GTY */
extern struct node * GTY(()) global_tree_root;
extern edge_t * GTY((length("global_edge_count"))) global_edges[];

/* Function pointer type with complex GTY attributes */
typedef int (* GTY((skip)) node_visitor)(
    struct node *current,
    void * GTY((skip)) context,
    int (* GTY((skip)) should_continue)(int, void *)
);

#endif /* TEST_GTY_H */
