/* complex_gty_test.c - Test file for gengtype delimiter parsing coverage */

/* Include necessary headers for GTY annotations */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void (*GTY((user)) complex_func_ptr)(
    int (*GTY((skip)) nested_callback)(char GTY((length("strlen(param)+1"))) param[]),
    struct GTY(()) inner_struct { 
        int x; 
        void (*GTY((chain_next)) another_fn)(double);
    }
);

/* Test 2: Array with complex dimension expression containing parentheses */
struct GTY(()) array_test {
    int GTY((tag("0"))) arr1[(10 + sizeof(struct GTY(()) temp { int a; }))];
    char *GTY((length("((n * 2) + 1)"))) arr2[];
};

/* Test 3: Deeply nested structure with all delimiter types */
struct GTY(()) outer_struct {
    /* Case '(': Function pointer with complex signature */
    int (*GTY((user)) complex_op)(
        struct GTY(()) param1 { 
            int values[5]; 
        },
        union GTY(()) param2 {
            long (*funcs[3])(void);
            struct { short s; } nested;
        } GTY((desc("%1.nested.s")))
    );
    
    /* Case '[': Multi-dimensional array with nested array dimensions */
    double matrix[3][(sizeof(int) * 2)][10];
    
    /* Case '{': Nested initializer-style structure */
    struct GTY(()) config {
        struct {
            int flags;
            struct GTY(()) options {
                char *GTY((length("len"))) name;
                int values[];
            } opts[2];
        } groups[3];
    } settings;
};

/* Test 4: Union with all delimiter types mixed */
union GTY(()) mixed_delimiters {
    /* Parentheses in function pointer array */
    void (*GTY((user)) handlers[5])(
        int arg1,
        char *GTY((length("arg2_len"))) arg2[]
    );
    
    /* Brackets in nested array type */
    struct GTY(()) nested_array {
        int (*GTY((skip)) ptr_array[2][3])(void);
    } na;
    
    /* Braces in anonymous struct */
    struct {
        struct GTY(()) inner {
            union {
                int x;
                long y;
            } u;
        } GTY((desc("%1.u.x"))) items[4];
    };
};

/* Test 5: Template-like macro expansion with delimiters */
#define DECLARE_CALLBACK(type, name) \
    type (*GTY((user)) name)(type (*GTY((skip)) converter)(type[], int), \
                             struct { type min; type max; } range)

struct GTY(()) template_test {
    DECLARE_CALLBACK(int, int_callback);
    DECLARE_CALLBACK(double, double_callback);
};

/* Test 6: Chain of structures with nested delimiters */
struct GTY((chain_next("next"), chain_prev("prev"))) chain_node {
    struct GTY(()) data {
        /* Complex array with size in parentheses */
        unsigned char buffer[(256 * sizeof(void *))];
        
        /* Function pointer with attributes */
        void (*GTY((user)) cleanup)(
            struct chain_node *GTY((skip)) node,
            int error_code
        );
    } payload;
    
    /* Self-referential with nested type */
    struct chain_node *GTY((skip)) next;
    struct chain_node *GTY((skip)) prev;
    
    /* Anonymous union with nested struct */
    union {
        struct GTY(()) metadata {
            int id;
            char *GTY((length("name_len"))) name;
            struct {
                time_t created;
                time_t modified;
            } timestamps;
        } meta;
        long raw_data[8];
    };
};

/* Test 7: Attribute list with nested parentheses */
typedef struct GTY(()) attribute_test {
    int GTY((tag("1"), 
             length("count"), 
             desc("%0.member"))) *dynamic_array;
    
    /* GTY attribute with nested function call-like syntax */
    void *GTY((user, 
               param_is(struct GTY(()) param_type { 
                   int version; 
                   char *name; 
               }))) user_data;
} attr_test_t;

/* Test 8: Multiple balanced delimiters in single declaration */
struct GTY(()) ultimate_test {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (*GTY((user)) fn_array[5])(
        int GTY((length("dim1"))) matrix[][10],
        struct GTY(()) context {
            int (*GTY((skip)) methods[3])(void);
            union {
                char str[100];
                void *ptr;
            } data;
        } *ctx
    );
    
    /* Initializer-like nested braces */
    struct GTY(()) config_block {
        struct {
            int flags;
            struct {
                short width;
                short height;
            } dimensions[2];
        } sections[3];
    } blocks[2];
};

/* Test 9: Recursive type definition with delimiters */
typedef struct GTY(()) tree_node tree_node_t;

struct GTY(()) tree_node {
    int value;
    
    /* Function pointer operating on tree nodes */
    void (*GTY((user)) visitor)(
        tree_node_t *GTY((skip)) node,
        void (*GTY((skip)) action)(int, tree_node_t *[])
    );
    
    /* Array of child nodes */
    tree_node_t *GTY((length("child_count"))) children[];
    
    /* Nested structure for metadata */
    struct GTY(()) node_info {
        int depth;
        struct {
            int is_leaf;
            int has_siblings;
        } flags;
    } info;
};

/* Test 10: Complex GTY annotation with all delimiters */
struct GTY((chain_next("nxt"),
            chain_prev("prv"),
            user,
            desc("((%0.type << 16) | %0.subtype)"))) annotated_struct {
    int type;
    int subtype;
    
    /* Field with GTY attributes containing parentheses */
    char *GTY((length("((len + 7) & ~7)"))) padded_string;
    
    /* Nested anonymous struct with array */
    struct {
        int counts[5];
        struct GTY(()) nested {
            void (*callback)(int[10]);
        } ns[2];
    } container;
    
    struct annotated_struct *nxt;
    struct annotated_struct *prv;
};

/* Main function to make the file compilable (though GTY expands to nothing normally) */
int main(void) {
    /* Dummy usage to avoid compiler warnings */
    struct outer_struct os = {0};
    struct mixed_delimiters md = {0};
    struct chain_node cn = {0};
    struct ultimate_test ut = {0};
    
    (void)os;
    (void)md;
    (void)cn;
    (void)ut;
    
    return 0;
}
