#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define VARRAY(type, size) type GTY((length(size))) []
#define NESTED_PTR(type) type * GTY((skip))

/* GTY attribute with string literal containing special characters */
#define DESC_ATTR(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant_node)))

/* Forward declarations */
struct tree_node;
struct variant_node;
union nested_union;

/* Complex recursive structure with multiple GTY annotation styles */
struct GTY(()) tree_node {
    int value;
    
    /* Parentheses for function pointer with GTY skip attribute */
    int (* GTY((skip)) traverse_callback)(struct tree_node *node, int depth);
    
    /* Brackets for array bounds - nested array declaration */
    struct tree_node * GTY((length("child_count"))) children[];
    
    /* Using macro expansion for complex type */
    PTR_ARRAY(struct tree_node) grandchildren;
    
    /* Union inside structure with GTY tag */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct tree_node * GTY((tag("1"))) next;
    } GTY((desc("%0.tag"))) variant;
    
    /* Nested structure with its own GTY markers */
    struct {
        int metadata;
        char * GTY((skip)) name;
        union nested_union * GTY((chain_next("%h.next"))) union_ptr;
    } GTY((cache)) info;
};

/* Union type with GTY conditional attributes */
union GTY((desc("%1.type"), param_is(struct variant_node))) nested_union {
    int int_val;
    double double_val;
    struct tree_node * GTY((tag("1"))) node_ptr;
    void (* GTY((skip)) func_ptr)(int, char *);
};

/* Variant structure with complex GTY attributes */
struct GTY((for_user)) variant_node {
    int type;
    
    /* Array with variable bounds */
    int GTY((length("type == 0 ? 10 : 20"))) values[];
    
    /* Function pointer with explicit argument list in parentheses */
    void (* GTY((skip)) processor)(
        struct variant_node *self,
        int (* GTY((skip)) comparator)(int, int)
    );
    
    /* Nested anonymous struct with GTY */
    struct {
        char * GTY((string)) str_data;
        int GTY((length("str_len"))) *int_array;
    } GTY((desc("anonymous struct"))) data;
};

/* Template-like structure using macros */
typedef struct GTY(()) template_node {
    int id;
    
    /* Multiple levels of indirection with GTY */
    struct tree_node ** GTY((length("id * 2"))) node_matrix[];
    
    /* Complex function pointer type */
    struct variant_node * (* GTY((skip)) factory)(
        int count,
        struct template_node (* GTY((skip)) templates)[]
    );
    
    /* Union with nested GTY-tagged pointers */
    union {
        int *int_ptr;
        struct {
            char * GTY((string)) buffer;
            size_t GTY((skip)) size;
        } GTY((desc("buffer_data"))) buf_data;
    } GTY((desc("%0.type == 0 ? \"int\" : \"buffer\""))) storage;
} template_node_t;

/* Global GTY variables */
extern struct tree_node * GTY(()) global_tree_root;
extern template_node_t * GTY((length("global_count"))) global_templates[];

/* Function declarations */
struct tree_node * GTY((returns_nonnull)) create_tree(int depth);
int traverse_tree(struct tree_node *root, 
                  int (* GTY((skip)) callback)(struct tree_node *, int));

#endif /* TEST_GTY_H */
