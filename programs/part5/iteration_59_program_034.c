#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_ARRAY(dim1, dim2) GTY((length("%0.dim1"))) GTY((length("%0.dim2"))) []

/* Conditional GTY attributes with string literals containing special chars */
#define DESC_ATTR(field) GTY((desc("\"%0." #field "\""), param_is(struct node)))
#define TAG_ATTR(tag_val) GTY((tag("\"" #tag_val "\"")))

/* Complex recursive structure with all delimiter types */
struct node GTY(())
{
    int value;
    
    /* Parentheses: Function pointer with GTY skip attribute */
    int (* GTY((skip)) callback)(
        struct node *child, 
        int depth
    );
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            float x;
            float y;
        } GTY((skip)) coordinates;
    } variant;
    
    /* Using macro with nested brackets */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Multi-dimensional array annotation */
    int matrix NESTED_ARRAY(rows, cols);
    
    int child_count;
    int len;
    int rows;
    int cols;
};

/* Union type with complex GTY attributes */
union tree_node GTY(())
{
    struct node * GTY((desc("%0.ptr"), param_is(struct node))) ptr;
    int GTY((desc("%0.value"))) value;
    float GTY((desc("%0.real"))) real;
    
    /* Nested structure inside union */
    struct {
        char * GTY((length("str_len"))) string;
        int str_len;
    } GTY((desc("%0.str"))) str_data;
};

/* Typedef with GTY marker */
typedef struct node * GTY(()) node_ptr;

/* Global variable declarations */
extern struct node * GTY(()) root_node;
extern union tree_node GTY(()) global_tree;

/* Function pointer type with complex signature */
typedef void (* GTY((skip)) traverse_func)(
    struct node *root,
    int (* GTY((skip)) visit)(struct node *, void *),
    void * GTY((skip)) context
);

#endif /* TEST_GTY_H */
