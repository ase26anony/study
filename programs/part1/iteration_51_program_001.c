/* Primary test header for gengtype parser coverage */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* ==================== */
/* Complex Type Definitions with Nested Balanced Tokens */
/* ==================== */

/* Test 1: Function pointer with deeply nested parameter lists */
typedef int (*complex_func_ptr)(
    int (*inner_callback)(char[10], int (*)(void)), 
    struct { 
        int a; 
        double b[(sizeof(int) > 2) ? 5 : 10]; 
    }
);

/* Test 2: GTY annotation with nested balanced tokens */
GTY((user, 
     ptr_alias("node_ptr"), 
     desc("tag::0")))
typedef struct node {
    struct node * GTY((skip)) next;
    int data[5][(sizeof(long) > 4) ? 8 : 4];
    char * GTY((length("len"))) buffer;
} node_t;

/* Test 3: Union containing struct with arrays and bit-fields */
typedef union {
    struct {
        unsigned int flags : 8;
        int matrix[3][(10 + 2)];
        char name[32];
    } s;
    long long int raw[4];
} nested_union_t GTY(());

/* Test 4: Multi-dimensional array with parenthesized size expressions */
typedef int complex_matrix_t[
    5][
    (sizeof(void*) == 8) ? 16 : 8
    ][
    (1 << 3)
];

/* Test 5: GCC attributes with balanced parentheses */
typedef int __attribute__((
    aligned(16), 
    packed, 
    deprecated("Use new_type instead")
)) aligned_int_t;

/* Test 6: Function pointer with attributes */
typedef void (__attribute__((
    stdcall, 
    noinline
)) *api_function_ptr)(
    int param1, 
    char param2[(10)]
);

/* Test 7: Nested GTY annotations */
GTY((chain_next("next"), chain_prev("prev")))
struct linked_list {
    struct linked_list *next;
    struct linked_list *prev;
    int values[5][(2 * 3)];
    GTY((skip)) void *user_data;
};

/* Test 8: Complex typedef with multiple levels of nesting */
typedef struct outer {
    int (*comparator)(
        const void *, 
        const void *, 
        int options[(5)]
    );
    union {
        struct {
            char * GTY((length("strlen(name) + 1"))) name;
            int scores[10][20];
        };
        void *raw_data[100];
    } data;
} outer_t;

/* Include macro-based tests */
#include "test_macros.h"

#endif /* TEST_GTY_H */
