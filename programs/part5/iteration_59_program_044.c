#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested_ptr)) *[]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Complex GTY attributes with string literals and special characters */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_DESC(len) GTY((desc("array[%d]", len), length(#len)))

/* Forward declarations */
struct node;
struct container;

/* Function pointer type with GTY annotation */
typedef CALLBACK_TYPE(int, (traverse_fn)(struct node *n, int depth, void *data)) traverse_callback;

/* Primary recursive structure with deeply nested GTY annotations */
struct GTY(()) node {
    int value;
    
    /* Parentheses: Function pointer with explicit argument list */
    traverse_callback GTY((skip)) callback;
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct container * GTY((tag("1"))) container;
    } variant;
    
    /* Using macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array with nested brackets */
    struct node ** GTY((length("grandchild_count"))) *nested_children[];
    
    int child_count;
    int grandchild_count;
};

/* Container structure with multiple GTY annotation styles */
struct GTY(()) container {
    /* Nested structure definition inside GTY */
    struct {
        int id;
        char * GTY((desc("name"))) name;
    } header;
    
    /* Array of arrays - nested brackets */
    struct node * GTY((length("array_size"))) node_array[][4];
    
    /* Union with GTY-tagged alternatives */
    union {
        int int_val;
        double GTY((tag("1"))) double_val;
        struct node * GTY((tag("2"), chain_next("%h.next"))) node_ptr;
    } GTY((desc("%0.type"))) data;
    
    /* Function pointer array with parentheses */
    traverse_callback GTY((length("callback_count"))) callbacks[];
    
    int array_size;
    int callback_count;
    struct container *next;
};

/* Global variables with GTY annotations */
extern struct node * GTY(()) root_node;
extern struct container * GTY((chain_next("%h.next"))) container_list;

/* Template-like structure using macros */
typedef struct GTY(()) graph {
    struct node * GTY((length("node_count"))) nodes[];
    struct container ** GTY((length("container_count"))) containers[];
    int node_count;
    int container_count;
    
    /* Complex attribute with string literal containing special chars */
    char * GTY((desc("graph::%s[id=%d]", "test_graph", 42))) name;
} graph_t;

#endif /* TEST_GTY_H */
