#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] depth
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip))) args

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Complex recursive structure with all delimiter types */
struct node GTY(())
{
    int value;
    
    /* Parentheses: Function pointer with explicit argument list */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next_node;
    } variant;
    
    /* Macro-expanded nested array */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Function pointer array with complex signature */
    void (* GTY((skip)) handlers[3])(struct node *n, int (*)(int));
};

/* Union with complex GTY annotations */
union tree_node GTY(())
{
    struct {
        int code;
        union tree_node * GTY((desc("%0.code"))) left;
        union tree_node * GTY((desc("%0.code"))) right;
    } expr;
    
    struct {
        char * GTY((length("len+1"))) name;
        int (* GTY((skip)) validate)(union tree_node *node, 
                                     int options[3],
                                     void (*callback)(void));
    } decl;
};

/* Template-like structure using macros */
struct container GTY(())
{
    int len;
    int child_count;
    
    /* Multiple levels of nested GTY attributes */
    struct node * GTY((chain_next("%h.next"), 
                       chain_prev("%h.prev"),
                       desc("%0.variant.tag"))) nodes;
    
    /* Array of function pointers with nested parentheses */
    int (** GTY((skip)) operations)(struct container *c, 
                                    int param,
                                    struct node *array[]);
    
    /* Nested structure with its own GTY marker */
    struct {
        int id;
        char * GTY((length("id % 10 + 1"))) buffer;
        struct container * GTY((skip)) parent;
    } metadata GTY(());
};

/* Global variable declarations */
extern struct node * GTY(()) root_node;
extern union tree_node * GTY(()) global_tree;

#endif /* TEST_GTY_H */
