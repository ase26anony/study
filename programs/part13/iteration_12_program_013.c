/* complex_gty_test.c - Test file for exercising gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with deeply nested parameter lists */
typedef void (*GTY(()) complex_fn_type)(
    int (*GTY(()) nested_callback)(
        char GTY(()) multi_array[][10][20],
        struct GTY(()) inner_struct {
            int x;
            double y[5];
        } *arg
    ),
    union GTY(()) data_union {
        int ival;
        void (*GTY(()) func_ptr)(int, char);
    } u
);

/* Test 2: Array with complex dimension expressions */
struct GTY(()) outer_container {
    /* Array dimension with nested parentheses */
    int GTY(()) arr1[(10 + (sizeof(struct GTY(()) temp { int a; double b; })))];
    
    /* Multi-dimensional array with function pointer elements */
    void (*GTY(()) fn_array[3][2])(
        int GTY(()) param_array[][(5 + 3)],
        struct GTY(()) another {
            char * GTY((length("len"))) str;
            int len;
        }
    );
    
    /* Nested struct with bit-fields and arrays */
    struct GTY(()) deeply_nested {
        unsigned int flags : 5;
        int GTY(()) matrix[4][(2 * 3)];
        struct GTY(()) inner_most {
            void (*GTY(()) callback[2])(
                int GTY(()) x,
                char GTY(()) buf[][10]
            );
        } innermost;
    } nested;
};

/* Test 3: Union containing various balanced delimiter constructs */
union GTY(()) complex_union {
    /* Function pointer returning pointer to array */
    int (*GTY(()) (*get_array_ptr)(void))[10];
    
    /* Struct with initializer-like designators */
    struct GTY(()) with_designators {
        int GTY(()) values[5];
        struct GTY(()) point {
            int x, y;
        } points[3];
    } data;
    
    /* Anonymous union within union */
    union {
        long GTY(()) big_array[100];
        struct GTY(()) small {
            char GTY(()) tiny[1];
        };
    };
};

/* Test 4: Template-like pattern using nested types */
struct GTY(()) node {
    struct node * GTY((skip)) next;
    struct node * GTY((skip)) prev;
    
    /* Complex type involving all three delimiters */
    void (*GTY(()) operations[5])(
        struct GTY(()) params {
            int GTY(()) count;
            char GTY(()) buffer[256];
        } args[],
        int GTY(()) flags
    );
    
    /* Nested switch-case like initializer */
    struct GTY(()) config {
        int GTY(()) mode;
        union GTY(()) settings {
            struct GTY(()) basic { int a; char b[10]; };
            struct GTY(()) advanced { void (*func)(int[5]); double matrix[3][3]; };
        } current;
    } cfg;
};

/* Test 5: Multiple balanced delimiters in single declaration */
static struct GTY(()) test_all_delimiters {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (*GTY(()) fn_array[5])(int GTY(()) param[][10]);
    
    /* Complex initializer with nested braces */
    struct GTY(()) initialized = {
        .fn_array = { NULL, NULL, NULL, NULL, NULL },
        .nested = {
            .flags = 0,
            .matrix = { {1, 2, 3}, {4, 5, 6} },
            .innermost = {
                .callback = { NULL, NULL }
            }
        }
    };
    
    /* Array with designators */
    int GTY(()) designated[10] = { [0] = 1, [5] = 2, [9] = 3 };
    
    /* Function pointer with attribute in parameter */
    void (*GTY(()) attr_func)(
        __attribute__((aligned(16))) int *ptr,
        char GTY(()) str[]
    );
} GTY(()) global_var;

/* Test 6: Recursive structures with function pointers */
struct GTY(()) tree_node {
    int GTY(()) value;
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
    
    /* Comparator function with complex signature */
    int (*GTY(()) compare)(
        const struct tree_node * GTY((skip)) a,
        const struct tree_node * GTY((skip)) b,
        void (*GTY(())) cleanup(
            char GTY(()) *data[],
            int GTY(()) count
        )
    );
    
    /* Array of function pointers returning function pointers */
    struct tree_node * (*GTY(()) (*find_methods[3])(void))(
        int GTY(()) criteria,
        char GTY(()) pattern[][20]
    );
};

/* Test 7: Multiple GTY annotations with nested attribute lists */
struct GTY(()) multi_annotation {
    /* Chain of pointers with multiple attributes */
    struct multi_annotation * GTY((chain_next("next"), chain_prev("prev"))) next;
    struct multi_annotation * GTY((chain_next("next"), chain_prev("prev"))) prev;
    
    /* Tagged union with nested GTY */
    union GTY((desc("tag"))) tagged {
        int GTY((tag("0"))) as_int;
        double GTY((tag("1"))) as_double;
        void (*GTY((tag("2"))) as_func)(
            struct GTY(()) context {
                int id;
                char name[50];
            } ctx
        );
    } GTY(()) data;
    
    /* Array of pointers with length attribute */
    char * GTY((length("len"))) strings[];
    int len;
};

/* Test 8: Extremely nested case */
typedef void (*GTY(())) (*nested_func_ptr_array[2][3])(
    int (*GTY(()) level1)(
        char (*GTY(()) level2[][5])(
            struct GTY(()) level3 {
                union GTY(()) level4 {
                    int (*GTY(()) level5)(void);
                    struct GTY(()) level6 { int x[10]; };
                } data;
            } param
        ),
        void (*GTY(()) callback)(int[][3][4])
    )
);

/* Test 9: Mixed delimiters in array dimensions and initializers */
struct GTY(()) mixed_delimiters {
    /* Array dimension with nested parentheses */
    int GTY(()) a[(sizeof(struct { int x; double y; }) + 10)];
    
    /* Initializer with nested braces and array designators */
    struct GTY(()) nested_init {
        int GTY(()) matrix[2][3] = { {1, 2, 3}, {4, 5, 6} };
        struct GTY(()) point {
            int x, y;
        } GTY(()) points[4] = { [0] = {1, 2}, [3] = {7, 8} };
    } GTY(()) init;
    
    /* Function pointer with array parameter and nested struct */
    void (*GTY(())) process(
        int GTY(()) data[][(2 + 3)],
        struct GTY(()) options {
            int GTY(()) flags : 8;
            char GTY(()) mode[20];
        } opts
    );
};

/* Test 10: Real-world like GCC tree structure simulation */
struct GTY(()) tree_common {
    enum tree_code code : 16;
    unsigned side_effects_flag : 1;
    unsigned constant_flag : 1;
    unsigned addressable_flag : 1;
    unsigned volatile_flag : 1;
    unsigned readonly_flag : 1;
    
    /* Complex type for tree operands */
    union GTY((desc ("TREE_CODE ((tree) &%h)"))) tree_node {
        struct GTY((tag ("0"))) tree_common common;
        struct GTY((tag ("1"))) tree_decl {
            struct tree_decl * GTY((skip)) arguments;
            tree GTY((length ("DECL_SIZE_UNIT (NODE)"))) initial;
        } decl;
        struct GTY((tag ("2"))) tree_type {
            tree GTY((skip)) values;
            tree GTY((skip)) maxval;
            tree GTY((skip)) minval;
        } type;
    } GTY((skip)) u;
};

/* Dummy main to make file compilable */
int main(void) {
    return 0;
}
