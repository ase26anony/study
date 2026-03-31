#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Template-like macro with nested brackets */
#define PTR_ARRAY(type) type * GTY((length("len"))) []

/* Macro with nested parentheses for function pointers */
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Complex GTY attributes with string literals */
#define VARIANT_ATTRS GTY((desc("%0.tag"), param_is(struct variant)))

/* Base structure with deeply nested annotations */
typedef struct base_node base_node;

struct base_node GTY(()) {
    int value;
    
    /* Parentheses: Function pointer with explicit argument list */
    int (* GTY((skip)) callback)(struct base_node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct base_node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct base_node * GTY((tag("1"))) node_ref;
    } variant;
};

/* Recursive tree structure using macros */
struct tree_node GTY(()) {
    int id;
    int child_count;
    
    /* Using macro with nested brackets */
    PTR_ARRAY(struct tree_node) grandchildren;
    
    /* Complex function pointer type with nested parentheses */
    CALLBACK_TYPE(int, (struct tree_node *self, void * GTY((skip)) context)) processor;
    
    /* Nested structure with its own GTY markers */
    struct {
        int depth;
        /* Array of pointers in nested struct */
        struct tree_node * GTY((length("2"))) level_children[2];
        
        /* Union with GTY attributes */
        union {
            char * GTY((tag("1"))) str_data;
            int * GTY((tag("2"))) int_array GTY((length("array_len")));
        } data GTY((desc("%0.depth")));
    } metadata;
    
    /* Pointer to same type (recursive) */
    struct tree_node * GTY((skip)) next;
};

/* Global variable with GTY marker */
extern struct tree_node * GTY(()) global_tree_root;

/* Function pointer type with complex signature */
typedef void (* GTY((skip)) 
    complex_handler)(struct tree_node *nodes[] GTY((length("count"))), 
                     int count,
                     void (* GTY((skip)) nested_cb)(int));

/* Union type with GTY markers on members */
union variant_data GTY(()) {
    /* Array in union */
    int numbers GTY((length("10")))[10];
    
    /* Nested structure in union */
    struct {
        char * GTY((skip)) name;
        /* Two-dimensional array */
        float matrix GTY((dim(3, 4)))[3][4];
    } structured;
    
    /* Function pointer in union */
    int (* GTY((skip)) compute)(int, int);
};

#endif /* TEST_GTY_H */
