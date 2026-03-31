#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip)) *
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional attribute macro */
#define VARIANT_ATTR(tagval) GTY((desc("%0.tag"), param_is(struct variant), tag(tagval)))

/* Base GTY structure with all delimiter types */
typedef struct base_node base_node;
typedef struct complex_node complex_node;

/* Union with GTY attributes inside */
union variant_data {
    int tag;
    void * VARIANT_ATTR("0") data;
    base_node * VARIANT_ATTR("1") node_ptr;
};

/* Structure with deeply nested GTY annotations */
struct base_node GTY(()) {
    int value;
    
    /* Parentheses: function pointer with complex signature */
    int (* GTY((skip)) 
        callback)(struct base_node *self, 
                  struct base_node * GTY((skip)) *children, 
                  int depth);
    
    /* Brackets: nested array with variable length */
    struct base_node * GTY((length("child_count"))) 
        *children[];
    
    /* Braces: embedded union */
    union {
        int simple_tag;
        union variant_data GTY((tag("1"))) complex;
    } variant;
    
    /* Macro-expanded array */
    PTR_ARRAY(struct base_node) grandchildren;
    
    /* Double pointer with skip */
    NESTED_PTR(struct base_node) parent_chain;
};

/* Recursive structure definition */
struct complex_node GTY(()) {
    struct base_node * GTY((skip)) base;
    
    /* Function pointer array with parentheses */
    void (* GTY((length("func_count"))) 
        handlers[])(struct complex_node *, 
                   int (*)(int, int));
    
    /* Nested structure with GTY */
    struct {
        int depth;
        struct complex_node * GTY((skip)) 
            * GTY((length("max_depth"))) 
            depth_nodes[];
    } GTY(())) recursion_info;
    
    /* Union with conditional attributes */
    union {
        char * GTY((tag("0"))) string;
        struct base_node * GTY((tag("1"))) node;
        struct complex_node * GTY((tag("2"))) complex;
    } GTY((desc("%0.type"))) data;
};

/* Global GTY variables */
extern struct base_node * GTY(()) global_tree_root;
extern struct complex_node ** GTY((length("global_count"))) 
    global_complex_nodes[];

/* Template-like structure with multiple nested levels */
struct nested_container GTY(()) {
    /* Triple pointer with array */
    struct base_node * GTY((skip)) 
        * GTY((length("container_size"))) 
        *containers[];
    
    /* Function pointer with array parameters */
    int (* GTY((skip)) 
        processor)(int matrix[][10], 
                  struct base_node *nodes[]);
    
    /* Anonymous struct with GTY */
    struct GTY(()) {
        int id;
        union variant_data * GTY((skip)) variants;
    } metadata;
};

#endif /* TEST_GTY_H */
