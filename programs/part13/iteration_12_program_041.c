/* complex-gty-test.c - Test file for gengtype delimiter parsing */

/* This file contains complex type definitions with nested parentheses,
   brackets, and braces to exercise the consume_balanced logic in
   gengtype-parse.cc lines 341-352. */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "gtype-desc.h"

/* Test 1: Function pointer with nested parameter list (parentheses) */
typedef void (*GTY(()) complex_func_ptr)(
    int (*GTY(()) nested_callback)(char GTY(()) buffer[256]),
    struct GTY(()) inner_struct { int x; } *arg2
);

/* Test 2: Array with complex dimension expression (brackets with parentheses) */
struct GTY(()) array_container {
    int GTY(()) multi_array[10][(5 + sizeof(struct GTY(()) temp { int a; }))];
    char GTY(()) variable_len[((int)sizeof(double) * 2)];
};

/* Test 3: Nested structure with all delimiter types */
struct GTY(()) outer_struct {
    /* Function pointer array */
    void (*GTY((chain_next, chain_prev)) func_array[5])(
        int param1,
        struct GTY(()) { 
            int data[10]; 
            union GTY(()) { 
                long l; 
                double d; 
            } u;
        } param2
    );
    
    /* Nested anonymous struct with initializer-like designators */
    struct GTY(()) {
        int GTY(()) matrix[3][3];
        struct GTY(()) {
            char * GTY((length("len"))) str;
            int len;
        } GTY(()) nested;
    } GTY(()) anonymous_member;
    
    /* Complex pointer declaration with attributes */
    struct GTY((desc("%1.var"), tag("1"))) tagged_union * GTY((skip)) next;
};

/* Test 4: Union with deeply nested delimiters */
union GTY(()) deep_nesting {
    struct GTY(()) level1 {
        int (*GTY(()) level2_fn)(
            char GTY(()) level3_arr[][10],
            union GTY(()) level3_union {
                struct GTY(()) { 
                    int x[5]; 
                } s;
                void (*GTY(()) f)(void);
            } *u
        );
        struct GTY(()) level2_struct {
            int data;
        } GTY(()) s;
    } GTY(()) l1;
    
    long GTY(()) simple;
};

/* Test 5: Typedef with template-like pattern (simulated with macros) */
#define GTY_TEMPLATE(name) struct GTY(()) name##_t

GTY_TEMPLATE(vector) {
    void ** GTY((length("%h.size"))) items;
    size_t size;
    size_t capacity;
};

/* Test 6: Structure with bit-field containing array-like syntax */
struct GTY(()) bitfield_test {
    unsigned int GTY(()) flags : (sizeof(int) * 8 - 1);
    struct GTY(()) {
        int GTY(()) count;
        char GTY(()) buf[(16 + 4)];
    } GTY(()) container;
};

/* Test 7: Multiple balanced delimiters in single declaration */
struct GTY(()) delimiter_combo {
    /* Contains: (*, ), [5], (, [][10], ) */
    void (*GTY(()) fn_array[5])(int GTY(()) param[][10]);
    
    /* Contains: {, }, [, ], =, {, } */
    struct GTY(()) {
        int GTY(()) values[3];
    } GTY(()) data = { .values = {1, 2, 3} };
    
    /* Complex function pointer with nested attributes */
    void (*GTY((chain_next("next"), chain_prev("prev"))) 
           complex_handler)(
               struct GTY(()) { 
                   int (*GTY(()) compare)(const void *, const void *); 
               } *ctx,
               void GTY(()) *args[]
           );
};

/* Test 8: Recursive structure with function pointer */
struct GTY(()) tree_node {
    int GTY(()) value;
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
    int (*GTY(()) visitor)(
        struct tree_node * GTY(()),
        void (*GTY(())) callback(int, char * GTY(()))
    );
};

/* Test 9: Union containing array of function pointers */
union GTY(()) multi_func {
    int (*GTY(()) int_funcs[3])(int, int);
    void (*GTY(()) void_funcs[2])(
        struct GTY(()) { 
            char * GTY((string)) name; 
        } *
    );
};

/* Test 10: Structure with nested initializer braces */
struct GTY(()) has_initializer {
    struct GTY(()) point {
        int x;
        int y;
    } GTY(()) points[4];
    
    /* This will generate nested braces during parsing */
    struct GTY(()) config {
        int GTY(()) settings[2][2];
        struct GTY(()) { 
            int depth; 
        } GTY(()) nested;
    } GTY(()) cfg;
};

/* Test 11: Complex typedef with all delimiters */
typedef struct GTY(()) {
    int (*GTY(()) processor)(
        char GTY(()) input[],
        int GTY(()) lengths[][5],
        struct GTY(()) { 
            void (*GTY(())) cleanup(void); 
        } *ctx
    );
    union GTY(()) {
        long GTY(()) l;
        double GTY(()) d[(sizeof(long) / sizeof(double)) + 1];
    } GTY(()) data;
} GTY(()) mega_type;

/* Test 12: Structure with attribute containing nested parentheses */
struct GTY((desc("%1.test"), 
           user("GTY((skip)) struct extra *ptr;"))) attr_test {
    int GTY(()) test;
    char GTY(()) name[(10 + 2 * (3 - 1))];
};

/* Test 13: Multiple levels of nested structures */
struct GTY(()) level_a {
    struct GTY(()) level_b {
        struct GTY(()) level_c {
            int (*GTY(())) func_in_c)(struct GTY(()) level_d {
                int data[5][5];
            } *);
            int GTY(()) array_in_c[10][(2 * (3 + 4))];
        } GTY(()) c;
        struct GTY(()) { 
            int x; 
        } GTY(()) anon;
    } GTY(()) b;
    struct level_a * GTY((chain_next)) next;
};

/* Test 14: Variable with complex array dimensions using sizeof */
struct GTY(()) size_based {
    char GTY(()) buffer[sizeof(struct GTY(()) { 
        double d; 
        int i; 
    }) * 2];
    int GTY(()) matrix[3][(sizeof(long double) + 7) / 8];
};

/* Test 15: Final comprehensive test with all delimiter types mixed */
struct GTY(()) ultimate_test {
    /* Parentheses in function type, brackets in array, braces in struct */
    union GTY(()) {
        /* Case '(' */
        int (*GTY(()) fp)(int (*(*GTY()))[5], ...);
        
        /* Case '[' */
        struct GTY(()) {
            int GTY(()) deep_array[3][4][(2 + 3)];
        } GTY(()) sa;
        
        /* Case '{' */
        struct GTY(()) {
            struct GTY(()) { 
                int x; 
            } GTY(()); 
        } GTY(()) sb;
    } GTY(()) u;
    
    /* Multiple attributes with parentheses */
    struct GTY((user("GTY((skip)) void *skip_me;"))) attr_struct * GTY(()) ptr;
};

/* Main function to make the file compilable */
int main() {
    return 0;
}
