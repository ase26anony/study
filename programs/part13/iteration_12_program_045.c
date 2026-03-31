/* complex_gty_test.c - Test file to exercise gengtype-parse.cc delimiter handling */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Complex function pointer with nested parentheses */
typedef void GTY(()) (*complex_func_ptr)(
    int (*GTY((callback)) nested_callback)(char GTY((length))[10]),
    struct GTY(()) { 
        int x; 
        union GTY(()) { 
            long a; 
            double b; 
        } u;
    } param
);

/* Test 2: Array with complex dimension containing parentheses */
struct GTY(()) ArrayTest {
    int arr1[(10 + sizeof(struct GTY(()) { char c; int i; }))];
    char * GTY((length)) arr2[5][(2 * sizeof(void*))];
};

/* Test 3: Nested structures with all delimiter types */
struct GTY(()) OuterStruct {
    /* Case '(': Function pointer field */
    int (*GTY((chain_next, chain_prev)) compare_fn)(
        const struct GTY(()) Inner {
            int values[10];
            struct GTY(()) { 
                char tag; 
                union GTY(()) { 
                    int num; 
                    char * GTY((ptr)) str; 
                } data;
            } variant;
        } *a,
        const struct Inner *b
    );
    
    /* Case '[': Multi-dimensional array with complex initializer */
    int matrix[3][(sizeof(int*) * 2)] GTY((default));
    
    /* Case '{': Nested anonymous struct */
    struct GTY(()) {
        struct GTY(()) {
            int depth;
            void (*GTY(()) handlers[5])(void);
        } nested;
        union GTY(()) {
            struct GTY(()) { int x; double y; } point;
            long coordinates[2];
        } location;
    } container;
};

/* Test 4: Typedef with deeply nested combinations */
typedef struct GTY(()) TreeNode {
    struct TreeNode * GTY((chain_next, chain_prev)) left;
    struct TreeNode * GTY((chain_next, chain_prev)) right;
    
    /* Complex member combining all delimiters */
    void (* GTY(()) operations[3])(
        int param1[(sizeof(struct GTY(()) { int a; char b; }))],
        struct GTY(()) {
            int (* GTY(()) validator)(char input[][10]);
            union GTY(()) {
                int codes[5];
                struct GTY(()) { int start; int end; } range;
            } check;
        } *config
    );
    
    /* Initializer with nested braces */
    struct GTY(()) {
        int flags;
        char * GTY((length)) name;
    } data = { 0, NULL };
} TreeNode;

/* Test 5: Union with variant types containing different delimiters */
union GTY(()) VariantType {
    /* Function pointer variant */
    int (* GTY(()) func_variant)(
        char * GTY((ptr)) args[],
        int count
    );
    
    /* Array variant with computed size */
    char array_variant[(sizeof(struct GTY(()) Header { 
        int size; 
        char magic[4]; 
    }) + 10)];
    
    /* Nested struct variant */
    struct GTY(()) {
        struct GTY(()) {
            int type;
            void (* GTY(()) callback)(void);
        } meta;
        union GTY(()) {
            long num;
            double real;
        } value;
    } struct_variant;
};

/* Test 6: Template-like pattern using macros (C-style) */
#define DECLARE_CONTAINER(TYPE) \
    struct GTY(()) Container_##TYPE { \
        TYPE * GTY((length)) items; \
        int (* GTY(()) compare)(TYPE a, TYPE b); \
        struct GTY(()) { \
            int capacity; \
            TYPE buffer[1]; \
        } storage; \
    }

/* Instantiate with complex type */
DECLARE_CONTAINER(struct GTY(()) {
    int id;
    char * GTY((ptr)) name;
    void (* GTY(())) methods[2](int);
});

/* Test 7: Multiple GTY annotations in single declaration */
struct GTY(()) MultiAnnot {
    /* Chain of pointers with annotations */
    struct MultiAnnot * GTY((chain_next)) next;
    struct MultiAnnot * GTY((chain_prev)) prev;
    
    /* Skip field */
    int GTY((skip)) internal_counter;
    
    /* Length field */
    char * GTY((length("len"))) dynamic_string;
    int len;
    
    /* Nested with attribute */
    struct GTY((for_user)) {
        int user_id;
        void (* GTY(()) user_callback)(
            struct GTY(()) { int code; char msg[100]; } *result
        );
    } user_data;
};

/* Test 8: Complex initializer triggering brace consumption */
static struct GTY(()) GlobalData = {
    .matrix = { {1, 2}, {3, 4} },
    .container = {
        .nested = {
            .depth = 5,
            .handlers = { NULL, NULL, NULL, NULL, NULL }
        },
        .location = {
            .point = { .x = 10, .y = 20.5 }
        }
    },
    .compare_fn = NULL
};

/* Test 9: Recursive type definition with all delimiters */
typedef struct GTY(()) Expr {
    enum { CONST, BINOP, CALL } type;
    union GTY(()) {
        /* Constant: number in parentheses (simulating cast) */
        struct GTY(()) {
            long value;
        } constant;
        
        /* Binary operation: operator between two expressions */
        struct GTY(()) {
            char op;
            struct Expr * GTY((ptr)) left;
            struct Expr * GTY((ptr)) right;
        } binop;
        
        /* Function call: function name and argument list */
        struct GTY(()) {
            char * GTY((ptr)) func_name;
            struct Expr * GTY((length)) args[];
        } call;
    } data;
} Expr;

/* Test 10: Edge case - empty balanced delimiters */
struct GTY(()) EmptyDelims {
    void (* GTY(()) empty_func)();
    int empty_array[];
    struct GTY(()) {} empty_struct;
};

/* Main function to make file compilable */
int main() {
    return 0;
}
