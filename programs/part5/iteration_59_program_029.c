#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) * GTY((length("depth"))) []

/* Conditional GTY attributes with string literals containing special characters */
#define DESC_ATTR(field) GTY((desc("\"%0." #field "\""), param_is(struct node)))
#define TAG_ATTR(tag_value) GTY((tag("\"" #tag_value "\"")))

/* Complex structure with all three delimiter types in GTY annotations */
struct GTY((desc("%0.tag"), param_is(struct node))) node {
    int tag;
    int value;
    
    /* Parentheses for function pointer with GTY skip attribute */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets for array bounds - nested array declaration */
    struct node * GTY((length("child_count"))) children[];
    
    /* Using macro expansion for complex array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Braces for nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next;
    } variant;
    
    /* Function pointer array with complex signature */
    void (* GTY((skip)) handlers[3])(struct node * GTY((skip)), int (*)(int));
    
    /* Nested structure with its own GTY annotation */
    struct GTY(()) metadata {
        char * GTY((length("strlen(name)+1"))) name;
        int * GTY((length("count"))) values;
        struct {
            int x;
            int y;
        } GTY((skip)) point;
    } meta;
};

/* Recursive tree structure with multiple pointer types */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) tree_node {
    int value;
    
    /* Array of pointers with variable bounds */
    struct tree_node * GTY((length("2 * level + 1"))) siblings[];
    
    /* Function pointer with nested parentheses in type */
    void (* GTY((skip)) traverse)(struct tree_node *root, 
                                  void (*visit)(struct tree_node *, void *),
                                  void *context);
    
    /* Union with tag-based discrimination */
    union GTY((desc("%1.tag"))) {
        int tag;
        struct tree_node * GTY((tag("TAG_NODE"))) child;
        char * GTY((tag("TAG_STRING"))) str;
        int (* GTY((tag("TAG_FUNC"))) func)(int, int);
    } data;
    
    /* Pointer to next node for linked list */
    struct tree_node *next;
    struct tree_node *prev;
};

/* Typedef with GTY annotation containing all delimiter types */
typedef struct GTY((for_user)) complex_type {
    /* Multi-dimensional array with GTY length attribute */
    int * GTY((length("dim1 * dim2 * dim3"))) multi_array[][3][2];
    
    /* Function pointer array with nested signatures */
    int (* GTY((skip)) operations[5])(struct complex_type *self, 
                                      int (*transform)(int, void *), 
                                      void * GTY((skip)) arg);
    
    /* Nested union with anonymous struct */
    union {
        struct {
            int x;
            int y;
        } GTY((skip)) coords;
        double matrix[2][2];
    } GTY((desc("%0.type"))) transform;
} complex_type_t;

/* Global variable declarations with GTY */
extern struct node * GTY(()) global_node_list;
extern complex_type_t * GTY((length("global_count"))) global_array[];

/* Macro for creating complex function pointer types */
#define COMPLEX_CALLBACK(name, ret, ...) \
    ret (* GTY((skip)) name)(__VA_ARGS__, void * GTY((skip)) context)

/* Structure using the complex callback macro */
struct GTY(()) callback_container {
    COMPLEX_CALLBACK(on_event, int, struct node *, int, char *);
    COMPLEX_CALLBACK(on_error, void, const char *, int);
};

#endif /* TEST_GTY_H */
