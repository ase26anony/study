/* Primary header file for gengtype parser coverage testing */
#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Include secondary headers to test multi-file parsing */
#include "test_nested.h"
#include "test_macros.h"

/* ==================== Complex Nested Type Definitions ==================== */

/* Test 1: Function pointer with deeply nested parameter lists */
typedef int (*complex_fp1)(int (*inner)(char[10]), 
                          struct {int a; int b;});

/* Test 2: Multi-dimensional array with parenthesized size expressions */
typedef int matrix_t[5][(sizeof(long) > 4) ? 10 : 20];

/* Test 3: Struct within union within typedef with arrays */
typedef union {
    struct {
        int x;
        char arr[5];
    };
    long l;
} nested_union_t;

/* Test 4: Even deeper nesting with all three bracket types */
typedef void (*ultra_complex_fp)(
    int (*level1)(char (*level2)[(2+3)], 
                 struct { 
                     union { 
                         int a[5]; 
                         char b[(10/2)]; 
                     } u; 
                 }),
    int matrix[3][(sizeof(int)*2)]
);

/* ==================== GTY Annotation Stress Testing ==================== */

/* Test 5: GTY markers with complex nested types */
typedef GTY((user)) struct node * GTY((skip)) node_ptr;

/* Test 6: GTY with chain_next and nested array */
GTY((chain_next = "next", chain_prev = "prev")) 
struct complex_list {
    struct complex_list *next;
    struct complex_list *prev;
    int data[(10+2)];
    char * GTY((tag("NAME"))) name;
};

/* Test 7: Multiple GTY attributes with nested structures */
typedef GTY((desc("tag"), 
            user, 
            length("count"))) 
struct tagged_array {
    int count;
    struct GTY((ptr_alias("node_ptr"))) node * GTY((length("count"))) items[];
} tagged_array_t;

/* Test 8: GTY within nested type definition */
struct outer_container {
    GTY((user)) struct inner {
        int x;
        char y[5];
    } inner_struct;
    int z;
};

/* ==================== Edge Cases with GCC Attributes ==================== */

/* Test 9: GCC attributes containing balanced parentheses */
typedef int __attribute__((aligned(16), 
                          packed, 
                          deprecated("use new_type instead"))) 
        aligned_int;

/* Test 10: Function pointer with stdcall attribute */
typedef void (*__attribute__((stdcall, 
                             noinline, 
                             warn_unused_result)) 
            api_fn)(int param1, 
                   char param2[(sizeof(int)*2)]);

/* Test 11: Nested attributes */
typedef struct __attribute__((aligned(32))) {
    int data __attribute__((packed, deprecated));
    char buffer[10] __attribute__((aligned(8)));
} attr_struct_t;

/* ==================== Macro Expansion Complexity ==================== */

/* Use macros from included header */
typedef GTY(()) PTR_TO(NESTED_ARRAY) complex_ptr;

/* Local macro definitions for additional testing */
#define ARRAY_DIM(X) [(X * 2)]
#define FUNC_PTR(RET, PARAMS) RET (*)(PARAMS)
#define NESTED_PARAMS int, char[5], struct {int a;}

/* Test 12: Complex macro expansion */
typedef GTY((user)) FUNC_PTR(int, NESTED_PARAMS) complex_func_ptr;

/* Test 13: Macro expanding to array with parenthesized dimension */
typedef int matrix2_t ARRAY_DIM(5) ARRAY_DIM((3+2));

/* Test 14: GTY arguments via macro */
#define GTY_CHAIN_ARGS (chain_next = "nxt", chain_prev = "prv")
struct GTY(GTY_CHAIN_ARGS) chained_list {
    struct chained_list *nxt;
    struct chained_list *prv;
    int value;
};

/* ==================== Additional Edge Cases ==================== */

/* Test 15: Empty balanced groups */
typedef struct empty_groups {
    int a;
    char b[];
} empty_groups_t;

/* Test 16: Mixed nested groups in single declaration */
typedef int (*(*nested_fp_arr[5])(int (*)(char[10])))(void);

/* Test 17: Bit-fields with complex expressions */
struct bitfield_test {
    unsigned int a : (sizeof(int) * 2);
    unsigned int b : ((10 > 5) ? 3 : 4);
    unsigned int c : 1;
};

#endif /* TEST_GTY_H */
