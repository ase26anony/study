#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [depth]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Complex GTY attributes with string literals and special characters */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_DESC(n) GTY((desc("array_" #n), length("%0.count")))

/* Forward declarations */
struct node;
struct tree;

/* Complex recursive structure with multiple GTY annotation styles */
struct GTY(()) node {
    int value;
    
    /* Parentheses for function pointer with explicit argument list */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets for array bounds - nested arrays */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces for nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next_node;
    } variant;
    
    /* Using macro expansion for template-like array declaration */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Nested array with variable bounds */
    int * GTY((length("value_count"))) values[];
    
    /* Function pointer with complex signature */
    CALLBACK_TYPE(void, (*complex_cb)(struct node **nodes[], int counts[3])) cb2;
};

/* Another structure with deeply nested GTY annotations */
struct GTY(()) tree {
    struct node * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) root;
    
    /* Array of pointers with nested GTY attributes */
    struct node ** GTY((length("%0.node_count"))) nodes[10];
    
    /* Union with GTY-tagged variants */
    union GTY((desc("%0.type"))) {
        int type;
        struct {
            int count;
            struct node * GTY((length("%0.count"))) items[];
        } GTY((tag("1"))) node_array;
        struct {
            char * GTY((length("%0.str_len"))) data;
        } GTY((tag("2"))) string_data;
    } data;
    
    /* Complex function pointer with multiple parameters */
    int (* GTY((skip)) traverse_fn)(
        struct tree *t,
        struct node *(* GTY((skip)) visitor)(struct node *, void *),
        void *context
    );
};

/* Typedef with GTY marker */
typedef struct GTY(()) list_node {
    struct list_node * GTY((skip)) next;
    struct list_node * GTY((skip)) prev;
    void * GTY((tag("0"))) data;
} list_node_t;

/* Global variable declarations with GTY markers */
extern struct tree * GTY(()) global_tree;
extern list_node_t * GTY(()) global_list;

/* Function pointer type with GTY skip */
typedef void (* GTY((skip)) cleanup_fn)(void *data, int flags);

/* Structure with conditional GTY attributes */
struct GTY(()) container {
    int capacity;
    
    /* Nested structure with GTY marker */
    struct GTY(()) item {
        int id;
        char * GTY((length("%0.name_len"))) name;
        struct item * GTY((skip)) next;
    } *items;
    
    /* Array of function pointers */
    cleanup_fn GTY((length("%0.handler_count"))) handlers[];
    
    /* Complex nested array declaration */
    int (* GTY((length("%0.matrix_size"))) matrix[][10]);
};

#endif /* TEST_GTY_H */
