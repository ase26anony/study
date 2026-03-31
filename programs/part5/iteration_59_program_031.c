#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("sub_len"))) * GTY((length("len"))) []
#define FUNC_PTR(ret, args) ret (* GTY((skip)) args)

/* GTY attributes with string literals containing special characters */
#define DESC_ATTR(field) GTY((desc("%0." #field), param_is(struct node)))
#define CONDITIONAL_ATTR GTY((desc("%0.tag == 1 ? \"ptr\" : \"int\""), param_is(struct variant)))

/* Primary recursive structure with all delimiter types */
struct node GTY(())
{
    int value;
    
    /* Brackets: Array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) callback)(struct node *child, int depth);
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct node * GTY((tag("1"))) next;
    } variant;
    
    /* Macro-expanded nested array */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Double pointer array using macro */
    NESTED_PTR_ARRAY(struct node) cousins;
    
    int child_count;
    int len;
    int sub_len;
};

/* Union type with conditional attributes */
union variant_data GTY(())
{
    int int_val;
    char * GTY((tag("1"))) str_val;
    struct node * GTY((tag("2"))) node_val;
    void (* GTY((tag("3"), skip)) func_val)(int, char *);
};

/* Structure with function pointer array */
struct callback_container GTY(())
{
    /* Array of function pointers with parentheses */
    void (* GTY((length("cb_count"), skip)) callbacks[])(struct node *, union variant_data);
    
    /* Nested structure with braces */
    struct {
        int priority;
        char * GTY((desc("%0.name"))) name;
    } metadata;
    
    int cb_count;
};

/* Template-like structure using all delimiter types */
struct complex_type GTY((desc("%0.complex_id")))
{
    int complex_id;
    
    /* Multi-dimensional array with brackets */
    int matrix[3][4];
    
    /* Pointer to array of function pointers */
    int (*(* GTY((skip)) operation_matrix[2])[3])(float, double);
    
    /* Deeply nested anonymous struct */
    struct {
        union {
            struct node * GTY((chain_next("%0.next"))) chain;
            struct {
                int (* GTY((skip)) validate)(struct complex_type *self, int flags);
                char flags[10];
            } config;
        } inner;
        
        /* Array of structures containing arrays */
        struct {
            int ids[5];
            char *names[];
        } items[2];
    } nested;
};

/* External declarations to force gengtype processing */
extern struct node * GTY(()) global_node_tree;
extern union variant_data GTY(()) global_variants[];
extern struct callback_container GTY(()) global_callbacks;

/* Typedef with GTY marker */
typedef struct node * GTY(()) node_ptr_t;
typedef int (* GTY((skip)) compare_func_t)(const struct node *, const struct node *);

#endif /* TEST_GTY_H */
