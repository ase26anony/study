#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested_ptr)) [][]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback)args

/* Complex GTY attributes with string literals and special characters */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_DESC(len) GTY((desc("array[%0." #len "]"), length(#len)))

/* Forward declarations */
struct node;
struct tree;

/* Complex recursive structure with all delimiter types */
struct GTY(()) node {
    int value;
    
    /* Parentheses: Function pointer with explicit argument list */
    int (* GTY((skip)) traverse_func)(struct node * GTY((skip)) self, 
                                     int (* GTY((skip)) visitor)(void *));
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next_node;
    } GTY((desc("%0.tag"))) variant;
    
    /* Using macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double-nested array */
    int GTY((length("dim1"), nested)) matrix[][10];
};

/* Tree structure with recursive pointer */
struct GTY(()) tree {
    struct node * GTY((skip)) root;
    
    /* Complex function pointer type with nested parentheses */
    void (* GTY((skip)) 
           complex_callback[2])(struct tree *t, 
                               int (*)(struct node **nodes[]));
    
    /* Union with GTY-tagged members */
    union {
        char * GTY((tag("0"))) str;
        struct node * GTY((tag("1"))) node_ptr;
        int (* GTY((tag("2"))) func_ptr)(int, char *[]);
    } GTY((desc("%0.type"))) data;
    
    /* Multi-dimensional array with GTY attributes */
    struct node * GTY((length("level_count"), 
                      nested_ptr)) level_nodes[][];
};

/* Typedef with GTY marker */
typedef struct GTY(()) list_node {
    void * GTY((skip)) data;
    struct list_node *next;
    /* Array of function pointers */
    int (* GTY((skip)) handlers[3])(void *data, int flags);
} list_node_t;

/* Global variable with GTY marker */
extern list_node_t * GTY((root)) global_list;

/* Template-like structure using macros */
struct GTY(()) container {
    int count;
    /* Macro expands to create complex token sequence */
    NESTED_PTR_ARRAY(struct node) node_grid;
    
    /* Function pointer with complex signature */
    CALLBACK_TYPE(int, (struct container *c, 
                       struct node *nodes[],
                       int (*filter)(struct node *))) processor;
};

/* Union with deeply nested GTY attributes */
union GTY((desc("%0.type"))) complex_union {
    int type;
    struct {
        int id;
        /* Nested structure with array */
        struct GTY(()) inner {
            char * GTY((length("str_len"))) name;
            int values[10];
        } *inner_ptr;
    } GTY((tag("1"))) structured;
    
    /* Array of pointers to functions returning pointers */
    void * (* GTY((tag("2"), skip)) func_array[5])(int, char *[]);
};

#endif /* TEST_GTY_H */
