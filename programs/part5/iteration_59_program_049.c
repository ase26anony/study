#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_len"))) * GTY((length("len"))) []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))

/* Complex recursive structure with all delimiter types */
struct node GTY(())
{
    int value;
    
    /* Brackets for array bounds - nested arrays */
    struct node * GTY((length("child_count"))) children[];
    
    /* Parentheses for function pointer with arguments */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Braces for nested union */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant GTY((desc("%0.tag")));
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array using macro */
    NESTED_PTR_ARRAY(struct node) cousins;
};

/* Union with complex GTY annotations */
union tree_node GTY(())
{
    struct {
        int type;
        union tree_node * GTY((skip)) left;
        union tree_node * GTY((skip)) right;
        
        /* Function pointer with complex signature */
        void (* GTY((skip)) traverse)(
            union tree_node *root,
            void (* GTY((skip)) visit)(union tree_node *, void *),
            void *context
        );
    } binary;
    
    struct {
        int count;
        /* Array of pointers with length attribute */
        union tree_node * GTY((length("%0.count"))) items[];
    } list;
    
    /* Embedded structure with its own GTY markers */
    struct embedded GTY(()) {
        char * GTY((length("strlen(%0)+1"))) name;
        int (* GTY((skip)) compare)(struct embedded *a, struct embedded *b);
    } embed;
};

/* Typedef with GTY marker */
typedef struct node * GTY((ptr)) node_ptr;

/* Global variable with GTY marker */
extern struct node * GTY((root)) global_tree_root;

/* Function pointer type with GTY skip */
typedef int (* GTY((skip)) node_visitor)(struct node *n, int depth);

#endif /* TEST_GTY_H */
