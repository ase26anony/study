/* Test file to exercise gengtype-parse.cc delimiter handling */
/* This should trigger consume_balanced for '(', '[', and '{' delimiters */

#include "gtype-desc.h"

/* Complex function pointer type with nested parentheses */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("strlen(%h)+1"))) buffer[256]),
    struct GTY((tag("1"))) { int x; double y; } param
);

/* Structure with deeply nested delimiters */
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) outer_struct {
    /* Case 1: Function pointer array with complex signature */
    int (*GTY((skip)) func_array[5])(
        int matrix[3][4],
        struct GTY(()) { char *name; int id; } config
    );
    
    /* Case 2: Nested structure with array of function pointers */
    struct GTY((for_user)) inner_struct {
        /* Array dimension with parentheses expression */
        int data[(10 + sizeof(struct GTY(()) { int a; char b; }))];
        
        /* Function pointer returning pointer to array */
        char (*(*GTY((skip)) get_string)(void))[256];
        
        /* Nested union with initializer-like braces in comments */
        union GTY((desc("%1.type"))) {
            int ival;
            double dval;
            char *GTY((length("%h.len"))) sval;
        } value;
    } inner;
    
    /* Case 3: Pointer to array of structures */
    struct GTY((reorder("resize_hook"))) {
        int count;
        /* Multi-dimensional array with computed size */
        float matrix[][(2 * 4 + 1)];
    } *GTY((length("%h.count"))) data_block;
    
    /* Self-referential pointer for chain */
    struct outer_struct *GTY((skip)) next;
    struct outer_struct *GTY((skip)) prev;
};

/* Union with complex nested types */
union GTY((tag("type"))) complex_union {
    /* Function pointer with attributes in parameter list */
    void (*GTY((skip)) operation)(
        __attribute__((aligned(16))) int *buffer,
        struct GTY(()) { int len; int cap; } meta
    );
    
    /* Array of pointers to functions returning structures */
    struct GTY((user)) result (*GTY((skip)) handlers[10])(
        int arg1,
        double arg2[10]
    );
    
    /* Nested structure containing array of unions */
    struct GTY((for_user)) {
        union GTY((desc("%1.utype"))) {
            int i;
            float f;
            char c[4];
        } values[100];
    } container;
};

/* Typedef with template-like pattern using nested parentheses */
typedef struct GTY((user)) tree_node *(*GTY((skip)) node_processor)(
    struct GTY((user)) tree_node *root,
    int (*GTY((skip)) visitor)(
        void *context,
        struct GTY((user)) tree_node *node,
        int level
    ),
    void *GTY((skip)) context
);

/* Structure with all delimiter types mixed together */
struct GTY((chain_next("%h.n"))) delimiter_test {
    /* Combination: pointer to function returning pointer to array */
    int (*(*GTY((skip)) complex_member)(
        int param1,
        char param2[]
    ))[10];
    
    /* Nested anonymous struct with bitfields */
    struct GTY(()) {
        unsigned int flag:1;
        unsigned int count:7;
        unsigned int:0;  /* padding */
        unsigned int values[4];
    } bits;
    
    /* Array of structures with function pointer members */
    struct GTY((user)) {
        char *name;
        int (*GTY((skip)) compare)(const char *, const char *);
        void (*GTY((skip)) cleanup)(void *);
    } callbacks[5];
    
    /* Multi-level pointer with array dimensions */
    int ***GTY((skip)) deep_ptr[2][3];
    
    struct delimiter_test *GTY((skip)) n;
};

/* Test case with initializer-like syntax in comments */
struct GTY((user)) with_init_style {
    int x;
    double y;
    /* This looks like an initializer but is just a comment */
    /* struct nested { int a = {1, 2, 3}; char b[] = "test"; }; */
    
    /* Actual member with array and nested struct */
    struct GTY(()) {
        int codes[10];
        struct GTY(()) { short s; long l; } pair;
    } data;
};

/* Edge case: function pointer returning function pointer */
typedef void (*(*GTY((skip)) signal_handler)(int signum))(void);

/* Structure using the signal_handler type */
struct GTY((user)) signal_handlers {
    signal_handler handlers[32];
    struct GTY(()) {
        int mask;
        void (*GTY((skip)) default_handler)(void);
    } config;
};

/* Macro to generate complex type (will be expanded before gengtype) */
#define DECLARE_VECTOR_TYPE(T) \
    struct GTY((user)) vector_##T { \
        T *GTY((length("%h.size"))) data; \
        size_t size; \
        size_t capacity; \
        void (*GTY((skip)) resize)(struct vector_##T *vec, size_t new_cap); \
    }

/* Instantiate the macro with complex type */
DECLARE_VECTOR_TYPE(struct GTY((user)) { int x; char *name; });

/* Another complex typedef */
typedef struct GTY((user)) {
    /* Nested array of function pointers with complex signatures */
    int (*GTY((skip)) operations[][5])(
        void *ctx,
        int args[],
        struct GTY(()) { int count; double *values; } *params
    );
    
    /* Union inside typedef */
    union GTY((desc("%1.tag"))) {
        int as_int;
        double as_double;
        struct GTY((user)) { char *str; size_t len; } as_string;
    } value;
} generic_processor;

/* Global variable declarations with GTY */
extern struct outer_struct *GTY((skip)) global_list;
extern generic_processor GTY((user)) global_processor;
