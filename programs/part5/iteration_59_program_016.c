#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Template-like macro with nested brackets */
#define PTR_ARRAY(type, len_field) type * GTY((length(#len_field))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_count"))) * GTY((length("count"))) []

/* Complex GTY attributes with string literals and parentheses */
#define CALLBACK_ATTR GTY((skip, desc("%0.callback")))
#define VARIANT_ATTR(tag_val) GTY((tag(#tag_val), desc("%0.variant_type")))

/* Forward declarations with GTY markers */
typedef struct tree_node tree_node;
typedef struct metadata metadata;

/* Union with GTY-tagged variants */
union variant_data {
    int tag;
    void * VARIANT_ATTR(0) ptr_data;
    int * VARIANT_ATTR(1) int_array;
    struct tree_node * VARIANT_ATTR(2) node_ref;
};

/* Primary recursive structure with all delimiter types */
struct GTY((
  desc("%0.value + %0.child_count"),
  param_is(struct tree_node)
)) tree_node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* CALLBACK_ATTR process_child)(
        struct tree_node * GTY((skip)) child,
        int depth,
        void * GTY((skip)) context
    );
    
    /* Brackets: Nested array with variable bounds */
    struct tree_node * GTY((length("child_count"))) * GTY((length("level"))) children[][];
    
    /* Braces: Nested union inside structure */
    union {
        int simple_type;
        union variant_data GTY((desc("%0.variant.tag"))) complex_variant;
        struct {
            char * GTY((length("str_len"))) name;
            int * GTY((length("data_len"))) data;
        } GTY((chain_next("%0.next"), chain_prev("%0.prev"))) named_data;
    } node_info;
    
    /* Using template-like macro expansion */
    PTR_ARRAY(struct tree_node, child_count) grandchildren;
    
    /* Double pointer array with GTY attributes */
    struct tree_node ** GTY((length("ref_count"), skip)) references;
    
    int child_count;
    int level;
    int str_len;
    int data_len;
    int ref_count;
};

/* Another structure with different GTY annotation style */
struct GTY((for_user)) metadata {
    /* Array of function pointers with parentheses */
    void (* GTY((length("callback_count"))) callbacks[])(
        struct tree_node *root,
        union variant_data *data
    );
    
    /* Nested structure with braces */
    struct {
        int version;
        char * GTY((length("path_len"))) path;
        struct tree_node * GTY((skip)) root_node;
    } GTY((desc("%0.config"))) config;
    
    /* Union with tag */
    union {
        int int_val;
        double dbl_val;
        struct tree_node * GTY((tag("1"))) node_val;
    } GTY((desc("%0.value_type"))) value;
    
    int callback_count;
    int path_len;
};

/* Global variable with GTY marker */
extern struct tree_node * GTY(()) global_tree_root;

/* Typedef with GTY */
typedef struct tree_node * GTY(()) node_ptr;

#endif /* GTY_TEST_H */
