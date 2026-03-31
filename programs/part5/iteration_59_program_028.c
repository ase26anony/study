#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested)) *[]
#define VARRAY(type, count) type GTY((length(#count))) []

/* Conditional GTY attributes with string literals containing special characters */
#define DESC_ATTR(field) desc("%0." #field)
#define PARAM_IS(type) param_is(struct type)

/* Complex type definitions with deeply nested GTY annotations */
typedef struct base_node base_node;

/* Function pointer type with GTY skip attribute */
typedef int (*node_callback_fn) GTY((skip)) (struct base_node *child, int depth);

/* Union with GTY tag */
union variant_data {
    int tag;
    void * GTY((tag("0"))) data;
    char * GTY((tag("1"))) str_data;
    struct base_node * GTY((tag("2"))) node_ptr;
};

/* Primary recursive structure with multiple GTY annotation styles */
struct base_node GTY((
    desc("%0.tag"),
    param_is(struct base_node)
)) {
    int tag;
    int value;
    
    /* Parentheses: Function pointer with GTY attribute */
    int (* GTY((skip)) callback)(struct base_node *child, int depth);
    
    /* Brackets: Array with variable bounds */
    struct base_node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int int_val;
        double GTY((skip)) double_val;
        char * GTY((length("str_len + 1"))) string_val;
        struct base_node * GTY((nested)) nested_node;
    } variant;
    
    /* Macro-expanded array type */
    PTR_ARRAY(struct base_node) grandchildren;
    
    /* Complex nested array with multiple dimensions */
    int GTY((length("dim1"), nested)) matrix[][10];
    
    /* Function pointer array */
    node_callback_fn GTY((length("callback_count"))) callbacks[];
};

/* Another structure with different GTY attribute combinations */
struct container GTY(()) {
    int id;
    
    /* String literal with escaped quotes in desc attribute */
    char * GTY((desc("name: \\\"%0.name\\\""))) name;
    
    /* Nested structure pointer with chain attribute */
    struct base_node * GTY((chain_next("%0.next"), chain_prev("%0.prev"))) first_node;
    
    /* Array of unions */
    union variant_data GTY((length("variant_count"))) variants[];
    
    /* Multi-dimensional pointer array */
    struct base_node * GTY((length("rows * cols"))) *node_grid[];
};

/* Template-like structure using macros */
struct complex_node GTY((
    desc("%0.type"),
    param_is(struct complex_node)
)) {
    char type;
    
    /* Macro with nested parentheses */
    NESTED_PTR_ARRAY(struct base_node) nested_children;
    
    /* Fixed-size array using VARRAY macro */
    VARRAY(int, 100) fixed_array;
    
    /* Conditional attribute based on type field */
    union {
        int * GTY((if("(%0.type == 'I')"))) int_ptr;
        double * GTY((if("(%0.type == 'D')"))) double_ptr;
        struct base_node ** GTY((if("(%0.type == 'N')"))) node_ptr_ptr;
    } data;
    
    /* Function pointer with complex signature */
    void (* GTY((skip)) 
           complex_func)(struct complex_node *self, 
                        int (* GTY((skip)) comparator)(const void *, const void *),
                        struct base_node * GTY((length("count"))) nodes[]);
};

/* Global variables with GTY annotations */
extern struct base_node * GTY((root)) global_root;
extern struct container * GTY((length("container_count"))) global_containers[];

#endif /* TEST_GTY_H */
