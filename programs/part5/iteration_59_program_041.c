#ifndef GTY_COMPLEX_TYPES_H
#define GTY_COMPLEX_TYPES_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len##depth"))) * GTY((length("len"))) []

/* Conditional GTY attributes with string literals containing special characters */
#define DESC_ATTR(field) GTY((desc("%0." #field), param_is(struct variant)))
#define SKIP_IF(cond) GTY((skip_if(#cond)))

/* Complex recursive structure with multiple GTY annotation styles */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses for function pointers */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets for array bounds - nested arrays */
    struct node * GTY((length("child_count"))) children[];
    
    /* Using macro expansion for complex type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Braces for nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant;
    
    /* Function pointer with complex signature */
    void (* GTY((skip)) complex_func)(
        struct node * GTY((skip)) nodes[],
        int (* GTY((skip)) filter)(int, char *),
        union { int x; double y; } GTY((skip)) param
    );
    
    struct node *next;
    struct node *prev;
};

/* Union with GTY attributes containing nested parentheses */
union GTY((desc("%0.tag"))) complex_union {
    int tag;
    
    /* Array with GTY annotation containing string literal */
    char * GTY((length("strlen(%0.data) + 1"))) data;
    
    /* Nested structure with its own GTY markers */
    struct {
        int count;
        struct node * GTY((length("count"))) items[];
    } GTY((tag("1"))) node_list;
    
    /* Function pointer array */
    int (* GTY((length("5"))) handlers[5])(int, char *);
};

/* Template-like structure using multiple macro expansions */
struct GTY(()) tree_container {
    /* Double pointer array with length attribute */
    struct node ** GTY((length("node_count"))) node_ptrs;
    
    /* Nested array of arrays */
    int GTY((length("dim1"))) matrix[][10];
    
    /* Union with conditional GTY attributes */
    union {
        int int_val;
        double GTY((skip_if("type_tag != 2"))) double_val;
        struct node * GTY((tag("3"))) node_ptr;
    } GTY((desc("%0.type_tag"))) data;
    
    /* Complex function pointer with nested parentheses */
    void (* GTY((skip)) (*signal_handler[3])(
        int sig,
        void (* GTY((skip)) old_handler)(int)
    ))(int);
};

/* Global variable with GTY marker */
extern struct node * GTY(()) global_node_list;

/* Typedef with GTY annotation */
typedef struct node * GTY(()) node_ptr_t;

#endif /* GTY_COMPLEX_TYPES_H */
