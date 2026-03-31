#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions that create nested delimiter sequences */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] depth
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip))) args

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))
#define ARRAY_LEN(field) GTY((length(#field)))

/* Complex function pointer type with nested parentheses */
typedef int (*complex_callback_t)(
    void * GTY((tag("0"))) data,
    int (* GTY((skip)) nested_callback)(int, char **),
    struct node * GTY((skip)) node
);

/* Primary recursive structure with deeply nested GTY annotations */
struct GTY((chain_next("next"), chain_prev("prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) callback)(
        struct node *child,
        int depth,
        char * GTY((skip)) name
    );
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union inside structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            char * GTY((skip)) str;
            int (* GTY((skip)) method)(int, int);
        } GTY((skip)) nested;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Another function pointer using CALLBACK_TYPE macro */
    CALLBACK_TYPE(int, (struct node *n, int (* GTY((skip)) inner)(void))) complex_handler;
    
    struct node *next;
    struct node *prev;
};

/* Union type with GTY markers containing all delimiter types */
union GTY((desc("%0.tag"))) variant_data {
    int GTY((tag("0"))) int_val;
    double GTY((tag("1"))) double_val;
    struct node * GTY((tag("2"), param_is(struct node))) node_ptr;
    
    /* Nested structure with array */
    struct {
        char * GTY((skip)) name;
        int values[10];
        struct node * GTY((length("count"))) items[];
    } GTY((tag("3"))) collection;
};

/* Template-like structure using multiple macro expansions */
struct GTY(()) tree {
    struct node * GTY((reorder("tree_reorder"))) root;
    
    /* Array of arrays */
    struct node * GTY((length("level_count"))) * GTY((length("node_count"))) levels[];
    
    /* Function pointer array */
    int (* GTY((length("callback_count"))) callbacks[])(
        union variant_data *data,
        int options[3],
        void (* GTY((skip)) cleanup)(void *)
    );
    
    /* Nested union with GTY attributes */
    union {
        int simple;
        struct {
            char * GTY((skip)) buffer;
            size_t (* GTY((skip)) allocator)(size_t, void * GTY((skip)) ctx);
        } complex;
    } GTY((desc("union_tag"))) storage;
};

/* Global variables with GTY markers */
extern struct node * GTY(()) global_node_list;
extern union variant_data GTY(()) global_variants[];
extern complex_callback_t GTY((skip)) global_callbacks[5];

/* Function declarations */
struct node * GTY((skip)) create_node(int value, int child_count);
void traverse_tree(struct node *root);
int calculate_sum(struct node *root);

#endif /* TEST_GTY_H */
