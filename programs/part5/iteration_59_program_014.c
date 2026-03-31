#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_len"))) * GTY((length("len"))) []
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* GTY structure with all three delimiter types */
struct GTY((desc("%0.tag"), param_is(struct node))) node {
    int value;
    int tag;
    
    /* Parentheses in function pointer */
    CALLBACK_TYPE(int, (struct node *child, int depth)) callback;
    
    /* Brackets for array bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Nested array via macro */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double-nested array */
    NESTED_PTR_ARRAY(struct node) great_grandchildren;
    
    /* Braces for nested union */
    union GTY((desc("%0.tag"))) {
        int int_val;
        char * GTY((tag("1"))) str_val;
        struct node * GTY((tag("2"))) node_val;
        void * GTY((tag("0"))) data;
    } variant;
    
    /* Another union with function pointers */
    union {
        int (* GTY((skip)) func1)(int, char **);
        void (* GTY((skip)) func2)(struct node * GTY((skip)), int []);
    } func_union;
    
    int child_count;
    int len;
    int sub_len;
};

/* Typedef with GTY marker */
typedef struct GTY(()) tree {
    struct node * GTY((skip)) root;
    int (* GTY((skip)) traverse)(struct tree *);
    struct tree * GTY((length("forest_size"))) forest[];
    int forest_size;
} tree_t;

/* Global variable with GTY */
extern tree_t * GTY(()) global_tree;

/* Function pointer type with complex signature */
typedef void (* GTY((skip)) complex_callback)(
    struct node * GTY((skip)), 
    int (* GTY((skip)) comparator)(const void *, const void *),
    char * GTY((length("str_len"))) []
);

#endif /* TEST_GTY_H */
