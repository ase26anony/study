#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Complex macro expansions with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [] depth
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) callback) args

/* Conditional GTY attributes with string literals */
#define VARIANT_DESC(tag_field) GTY((desc("%0." #tag_field), param_is(struct variant)))
#define ARRAY_LEN(field) GTY((length(#field)))

/* Complex recursive structure with all delimiter types */
typedef struct node node_t;

struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) node {
    int value;
    
    /* Parentheses: Function pointer with complex signature */
    int (* GTY((skip)) traverse_callback)(struct node *child, 
                                          int depth, 
                                          void * GTY((skip)) user_data);
    
    /* Brackets: Nested array with variable bounds */
    struct node * GTY((length("child_count"))) children[];
    
    /* Braces: Nested union within structure */
    union {
        int tag;
        void * GTY((tag("0"))) data;
        struct {
            char * GTY((string)) name;
            int (* GTY((skip)) processor)(int, char **);
        } GTY((desc("%1.tag"))) complex;
    } variant;
    
    /* Macro-expanded nested array */
    PTR_ARRAY(struct node) grandchildren;
    
    /* Function pointer with nested parentheses in args */
    CALLBACK_TYPE(int, (struct node *n, int (*)(int, int), char *[])) transformer;
    
    struct node *next;
    struct node *prev;
};

/* Union with nested GTY annotations */
union GTY((desc("%0.type"))) container {
    struct {
        int type;
        node_t * GTY((tag("1"))) nodes[10];
        struct {
            char * GTY((string)) id;
            int (* GTY((skip)) validators[3])(const char *, ...);
        } metadata;
    } structured;
    
    struct {
        int type;
        void * GTY((tag("2"))) raw_data;
        int (* GTY((skip)) (*nested_callback[2]))(int, ...);
    } raw;
};

/* Template-like structure with deeply nested attributes */
struct GTY(()) tree {
    node_t *root;
    
    /* Array with nested GTY attributes containing all delimiters */
    union container * GTY((length("container_count"),
                          param_is(union container),
                          desc("%1.structured.type"))) containers[];
    
    /* Function pointer array with complex signature */
    int (** GTY((skip)) operations[5])(
        struct tree *t,
        node_t * GTY((skip)) nodes[],
        int (*)(int, ...)
    );
    
    /* Nested structure with its own GTY markers */
    struct {
        int depth;
        char * GTY((string)) traversal_mode;
        int (* GTY((skip)) (*callback_chain[]))(
            void *,
            struct { int x; int y; } *
        );
    } GTY((desc("%0.depth"))) config;
};

/* Global GTY variables */
extern node_t * GTY((length("global_node_count"))) global_nodes[];
extern struct tree * GTY((skip)) global_tree;

/* Function pointer type with complex GTY attributes */
typedef int (* GTY((skip)) complex_handler_t)(
    struct node *,
    union container * GTY((skip)) containers[],
    int (*)(char * GTY((string)) [], ...)
);

#endif /* TEST_GTY_H */
