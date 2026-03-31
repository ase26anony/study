#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_len"))) * GTY((length("len"))) []
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* GTY structure with all delimiter types */
struct GTY((desc("%0.tag"), param_is(struct variant_node))) node {
    int value;
    
    /* Parentheses: function pointer with complex signature */
    int (* GTY((skip)) callback)(
        struct node *child, 
        int depth,
        void (* GTY((skip)) helper)(int, char **)
    );
    
    /* Brackets: nested arrays with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            char * GTY((string)) name;
            int (* GTY((skip)) methods[3])(void);
        } GTY((desc("%1.tag"))) obj;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array through macro */
    NESTED_PTR_ARRAY(struct node) cousins;
};

/* Recursive structure with mutual recursion */
struct GTY(()) tree {
    struct node * GTY((skip)) root;
    struct tree * GTY((skip)) left;
    struct tree * GTY((skip)) right;
    
    /* Union with nested GTY attributes */
    union GTY((desc("%0.type"))) {
        int ival;
        double fval;
        char * GTY((string)) sval;
        struct node * GTY((skip)) nodes[10];
    } value;
};

/* Typedef with GTY annotation containing all delimiters */
typedef struct GTY(()) {
    /* Function pointer array with parentheses */
    void (* GTY((skip)) handlers[5])(
        struct node *n,
        int arr[][10],
        union { int x; double y; } data
    );
    
    /* Nested structure with array of pointers */
    struct {
        struct node * GTY((length("count"))) items[];
        int (* GTY((skip)) validate[2])(
            char *str,
            int limits[3][2]
        );
    } GTY((chain_next("%0.next"))) container;
    
    /* Complex union with tag */
    union GTY((tag("TYPE"))) {
        struct node node;
        struct tree tree;
        void * GTY((skip)) ptr;
    } data;
} complex_type;

/* Global variables with GTY markers */
extern struct node * GTY(()) global_node_list[];
extern complex_type GTY(()) global_complex;

#endif /* TEST_GTY_H */
