#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_len"))) * GTY((length("len"))) []
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC GTY((desc("%0.tag"), param_is(struct variant)))
#define ARRAY_DESC(len) GTY((length(#len), desc("array_of_%0")))

/* Complex GTY-tagged structure with all delimiter types */
struct GTY(()) node {
    int value;
    
    /* Parentheses: function pointer with complex signature */
    int (* GTY((skip)) callback)(
        struct node *child, 
        int depth,
        void (* GTY((skip)) helper)(const char *)
    );
    
    /* Brackets: nested arrays with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            int x;
            int y;
            char * GTY((length("str_len"))) str;
        } GTY((desc("coord"))) coord;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array using macro */
    NESTED_PTR_ARRAY(struct node) cousins;
};

/* Union with GTY attributes containing all delimiter types */
union GTY((desc("%0.type"))) complex_union {
    int type;
    
    /* Case 1: Function pointer with parentheses */
    void (* GTY((skip)) func_ptr)(
        int a,
        int b[10],
        struct { int x; int y; } point
    );
    
    /* Case 2: Array with brackets */
    struct node * GTY((length("5"), desc("fixed_array"))) fixed[5];
    
    /* Case 3: Nested structure with braces */
    struct GTY((desc("nested_struct"))) {
        int id;
        char * GTY((length("name_len"))) name;
        union {
            int num;
            float arr[3];
        } data;
    } nested;
};

/* Typedef with GTY marker */
typedef struct GTY(()) tree {
    struct node * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) root;
    
    /* Complex attribute with string literal containing special chars */
    int (* GTY((skip, desc("compare_%0"))) compare)(
        const struct node *a,
        const struct node *b,
        void * GTY((skip)) context
    );
    
    /* Array of function pointers */
    void (* GTY((length("handler_count"))) handlers[])(
        struct tree *t,
        int options[2],
        const char *msg
    );
} tree_t;

/* Global variable with GTY marker */
extern tree_t * GTY(()) global_tree;

/* Function pointer type with GTY skip */
typedef int (* GTY((skip)) traverser_t)(
    struct node *current,
    int level,
    void * GTY((skip)) data,
    int results[]
);

#endif /* TEST_GTY_H */
