#ifndef GTY_TEST_HEADER_H
#define GTY_TEST_HEADER_H

/* Complex macro expansions that create nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] depth
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip))) args

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))
#define UNION_TAG(tag_val) GTY((tag(#tag_val)))

/* Base GTY structure with all three delimiter types */
struct GTY(()) base_node {
    int value;
    
    /* Parentheses for function pointer with complex signature */
    int (* GTY((skip)) callback)(struct base_node *self, 
                                 struct base_node * GTY((skip)) *children, 
                                 int depth);
    
    /* Brackets for array with nested type */
    struct base_node * GTY((length("child_count"))) children[];
    
    /* Braces for embedded union */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct base_node * GTY((tag("1"))) node_ref;
    } variant;
};

/* Recursive tree structure using macro expansions */
struct GTY(()) tree_node {
    int id;
    char * GTY((length("name_len"))) name;
    
    /* Using macro with nested brackets */
    PTR_ARRAY(struct tree_node) grandchildren;
    
    /* Complex function pointer with parentheses */
    CALLBACK_TYPE(void, (struct tree_node *node, 
                         int (* GTY((skip)) processor)(int), 
                         void *context)) visitor;
    
    /* Nested union with GTY attributes */
    union {
        struct {
            int * GTY((length("array_len"))) data_array;
            struct tree_node * GTY((skip)) left;
            struct tree_node * GTY((skip)) right;
        } binary;
        
        struct {
            struct tree_node * GTY((length("count"))) nodes[];
            int (* GTY((skip)) compare)(const void *, const void *);
        } list;
        
        struct {
            void (* GTY((skip)) action)(int, char * GTY((skip)) [], 
                                       struct tree_node * GTY((skip)));
            union {
                int int_val;
                double * GTY((skip)) double_ptr;
                struct tree_node * GTY((tag("2"))) complex_ref;
            } payload;
        } callback_wrapper;
    } GTY((desc("%0.type"))) node_type;
    
    /* Pointer to parent with chain length attribute */
    struct tree_node * GTY((chain_next("%0.next"), chain_prev("%0.prev"))) parent;
    struct tree_node * GTY((skip)) next;
    struct tree_node * GTY((skip)) prev;
};

/* Template-like structure with deeply nested GTY attributes */
struct GTY((param_is(struct param_type))) template_node {
    /* Array of function pointers with complex signatures */
    void (* GTY((length("callback_count"), skip)) callbacks[])(
        struct template_node *self,
        int (* GTY((skip)) filter)(int),
        void * GTY((skip)) user_data
    );
    
    /* Nested structure with its own GTY marker */
    struct {
        union {
            struct template_node * GTY((tag("TEMPLATE_CHILD"))) child;
            void ** GTY((length("ptr_count"), skip)) ptr_array;
        } GTY((desc("%0.union_tag"))) data;
        
        /* Multi-dimensional array */
        int (* GTY((skip)) matrix[3][3])(int, int);
    } nested;
    
    /* Variable length array of pointers to arrays */
    int * GTY((length("outer_len"))) * GTY((length("inner_len"))) jagged_array[];
};

/* Global GTY variables */
extern struct tree_node * GTY(()) global_tree_root;
extern struct base_node ** GTY((length("global_count"))) global_base_nodes;

/* Typedef with GTY annotation */
typedef struct tree_node * GTY((skip)) tree_handle;

/* Union type with conditional GTY attributes */
union GTY((desc("%0.type"))) variant_data {
    int int_val;
    char * GTY((length("str_len"))) string;
    struct tree_node * GTY((tag("TREE_NODE"))) node;
    void (* GTY((skip)) func_ptr)(union variant_data *);
    
    struct {
        int count;
        union variant_data * GTY((length("%0.count"))) items[];
    } array;
};

#endif /* GTY_TEST_HEADER_H */
