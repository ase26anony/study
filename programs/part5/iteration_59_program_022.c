#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip)) *
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Base GTY structure with multiple annotation styles */
typedef struct base_node GTY(())
{
    int value;
    /* Parentheses for function pointer with explicit args */
    int (* GTY((skip)) callback)(struct base_node *child, int depth);
    
    /* Brackets for array bounds - nested array declaration */
    struct base_node * GTY((length("child_count"))) children[];
    
    /* Braces for nested union */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct base_node * GTY((tag("1"))) node_ref;
    } variant;
} base_node_t;

/* Recursive tree structure using macro expansions */
struct tree_node GTY(())
{
    int id;
    int child_count;
    
    /* Using macro with brackets for array */
    PTR_ARRAY(struct tree_node) children;
    
    /* Nested pointer with skip attribute */
    NESTED_PTR(struct tree_node) parent_ref;
    
    /* Function pointer with complex signature */
    FUNC_PTR(int, (struct tree_node *self, int (*)(int)));
    
    /* Union with conditional attributes */
    union {
        int int_val;
        double dbl_val;
        char * GTY((tag("2"))) str_val;
        struct tree_node * GTY((tag("3"))) node_link;
    } GTY((desc("%0.tag"), param_is(union variant_data))) data;
    
    /* Another nested structure */
    struct {
        int depth;
        struct tree_node * GTY((skip)) leftmost;
        struct tree_node * GTY((skip)) rightmost;
    } bounds;
};

/* Template-like structure with multiple GTY annotations */
struct container GTY(())
{
    /* Array of pointers to arrays */
    struct tree_node * GTY((length("container_size"))) * GTY((skip)) node_matrix[];
    
    /* Function pointer array */
    void (* GTY((length("func_count"))) funcs[5])(struct container *, int);
    
    /* Nested structure with union */
    struct {
        enum { TYPE_INT, TYPE_PTR, TYPE_FUNC } type;
        union {
            int num;
            struct tree_node * GTY((tag("TYPE_PTR"))) ptr;
            int (* GTY((tag("TYPE_FUNC"))) func)(int, int);
        } GTY((desc("%0.type"))) value;
    } flexible;
};

/* Global GTY variables */
extern struct tree_node * GTY(()) global_tree_root;
extern struct container * GTY(()) global_container;

#endif /* TEST_GTY_H */
