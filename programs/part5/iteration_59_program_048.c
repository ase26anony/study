#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] GTY((depth))

/* Macro for function pointer with GTY skip attribute */
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback)args

/* Conditional GTY attributes with string literals containing special chars */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))

/* Complex recursive structure definition */
struct node GTY(())
{
    int value;
    
    /* Nested array with brackets - triggers consume_balanced('[') */
    struct node * GTY((length("child_count"))) children[];
    
    /* Double-nested array using macro */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Union with braces - triggers consume_balanced('{') */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) node_ref;
    } variant GTY((desc("%0.tag")));
    
    /* Function pointer with parentheses - triggers consume_balanced('(') */
    int (* GTY((skip)) traverse_fn)(struct node *self, int (* GTY((skip)) callback)(int));
    
    /* Another function pointer using macro */
    CALLBACK_TYPE(int, (struct node *child, int depth)) depth_callback;
    
    /* Pointer to array of function pointers (complex nesting) */
    int (** GTY((skip)) callbacks_array)(void);
    
    int child_count;
    int grandchild_len;
};

/* Even more complex structure with template-like patterns */
struct container GTY(())
{
    /* Array of structures containing unions with GTY attributes */
    struct {
        union {
            int ival;
            double dval;
            struct node * GTY((tag("2"))) nodeptr;
        } GTY((desc("%0.type"))) data;
        int type;
    } items[10];
    
    /* Nested structure definition inside GTY */
    struct inner GTY(()) {
        char * GTY((length("strlen+1"))) name;
        struct node ** GTY((length("count"))) nodes;
        int count;
        int strlen;
    } inner_struct;
    
    /* Function pointer with complex signature */
    struct node * (* GTY((skip)) find_node)(
        struct container *cont,
        int (* GTY((skip)) predicate)(struct node *),
        const char * GTY((skip)) name
    );
};

/* Typedef with GTY annotation containing all delimiter types */
typedef struct node * GTY((
    chain_next("%h.next"),
    chain_prev("%h.prev"),
    length("%0.child_count")
)) node_array_t;

/* Global variable with GTY annotation */
extern struct container * GTY(()) global_container;

/* Function declarations */
struct node *create_node(int value, int child_count);
int traverse_tree(struct node *root);
void process_container(struct container *cont);

#endif /* TEST_GTY_H */
