#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR(type) type * GTY((skip)) *
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals */
#define VARIANT_ATTR GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_ATTR(len) GTY((length(#len)))

/* Forward declarations with GTY markers */
typedef struct tree_node tree_node;
typedef struct complex_type complex_type;

/* Union with GTY-tagged members */
union variant_data {
    int tag;
    void * GTY((tag("0"))) data;
    char * GTY((tag("1"))) str;
};

/* Primary recursive structure with all delimiter types */
struct tree_node GTY((
  chain_next = "next",
  chain_prev = "prev",
  desc("tree_node %p")
)) {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) traverse_fn)(
        struct tree_node *root,
        int (* GTY((skip)) callback)(int, void *),
        void *user_data
    );
    
    /* Brackets: Nested array with variable bounds */
    struct tree_node * GTY((length("child_count"))) children[];
    
    /* Braces: Embedded union */
    union {
        int tag;
        struct tree_node * GTY((tag("0"))) left;
        complex_type * GTY((tag("1"))) right;
    } variant;
    
    /* Macro-expanded array */
    PTR_ARRAY(struct tree_node) grandchildren;
    
    struct tree_node *next;
    struct tree_node *prev;
};

/* Second structure with different nesting patterns */
struct complex_type GTY(()) {
    /* Function pointer array with parentheses */
    void (* GTY((length("fn_count"))) functions[])(
        struct complex_type *self,
        int arg1,
        char * GTY((skip)) arg2[]
    );
    
    /* Nested structure with GTY */
    struct {
        int depth;
        tree_node * GTY((skip)) nodes[10];
        union variant_data data;
    } GTY(())) container;
    
    /* Array of pointers to arrays */
    int * GTY((length("outer_len"))) * GTY((length("inner_len"))) matrix[];
};

/* Global variable with GTY marker */
extern struct tree_node * GTY(()) global_tree_root;

/* Typedef with GTY */
typedef struct tree_node * GTY(()) tree_ptr;

#endif /* TEST_GTY_H */
