#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Template-like macro with nested brackets */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("outer_len"))) * GTY((length("inner_len"))) []

/* Macro with parentheses for function pointers */
#define CALLBACK_TYPE(ret, ...) ret (* GTY((skip)) callback)(__VA_ARGS__)

/* Complex GTY attributes with string literals */
#define VARIANT_ATTR GTY((desc("%0.tag"), param_is(struct variant)))

/* Base structure with all three delimiter types */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) base_node {
    int value;
    struct base_node * GTY((skip)) next;
    struct base_node * GTY((skip)) prev;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) traverse_fn)(struct base_node *root, 
                                    int (* GTY((skip)) visitor)(void *data, int level),
                                    void * GTY((skip)) context);
    
    /* Brackets: Nested array declaration using macro */
    PTR_ARRAY(struct base_node) children;
    
    /* Braces: Embedded union */
    union {
        int tag;
        void * VARIANT_ATTR data;
        struct {
            char * GTY((length("str_len"))) name;
            int (* GTY((skip)) compare)(const char *a, const char *b);
        } GTY((tag("1"))) named;
    } variant;
};

/* Recursive tree structure with deeply nested annotations */
struct GTY(()) tree_node {
    int id;
    
    /* Multiple levels of nested brackets */
    struct tree_node * GTY((length("child_count"), 
                           reorder("tree_node_cmp"))) *children;
    
    /* Function pointer with parenthesized argument list containing brackets */
    void (* GTY((skip)) processor)(int matrix[3][3], 
                                   struct tree_node *nodes[],
                                   void (* GTY((skip)) callback)(int, char **));
    
    /* Union within struct with GTY attributes */
    union GTY((desc("%0.type"))) node_data {
        int ival;
        double dval;
        char * GTY((length("%0.slen"))) sval;
        struct tree_node * GTY((skip)) link;
        
        /* Nested struct in union */
        struct {
            int x;
            int y;
            int (* GTY((skip)) operation)(int, int);
        } coord;
    } data;
    
    /* Macro-expanded type with multiple bracket levels */
    NESTED_PTR_ARRAY(struct tree_node) grandchildren;
};

/* Typedef with GTY marker and function pointer */
typedef int (* GTY((skip)) complex_callback_t)(
    struct tree_node *root,
    int (* GTY((skip)) filter)(struct tree_node **nodes[], int count),
    char *options[]
);

/* Global variable declarations */
extern struct base_node * GTY((root)) global_tree_root;
extern complex_callback_t GTY((skip)) global_callback;

#endif /* TEST_GTY_H */
