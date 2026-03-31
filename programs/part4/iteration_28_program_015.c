/* test_expr_cc.c
 * 
 * This program is designed to trigger the uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization.
 * It uses various GNU C extensions and initialization patterns to
 * exercise different paths in the bounds checking logic.
 */

#include <stdio.h>
#include <stddef.h>

/* ==================== 1. Constant Array Bounds Initialization ==================== */

/* Use enum to define constant bounds */
enum { L = 2, H = 5 };
enum { BIG_START = 10, BIG_END = 90 };

/* ==================== 2. Target Condition Variation ==================== */

/* For count <= 2 path: small constant range */
static int small_range_array[10] = { [0 ... 1] = 42 };  /* count = 2, MEM_P(target)=true */

/* For count > 2 and MEM_P(target)=true path: large constant range */
static int large_array[100] = { [BIG_START ... BIG_END] = 99 };  /* count = 81 > 2 */

/* ==================== 3. Aggregate Types with Constant Sizes ==================== */

/* Packed struct with constant bitfield sizes */
struct __attribute__((packed)) PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
}; /* Total size = 19 bits, fits in uhwi */

/* Struct containing an array with constant bounds */
struct Container {
    int id;
    int data[8];
};

/* ==================== 4. Compiler Extensions ==================== */

/* Aligned array */
int aligned_array[16] __attribute__((aligned(64))) = { [3 ... 7] = 123 };

/* ==================== 5. Control Flow for Different Contexts ==================== */

void test_register_target(void) {
    /* Try to force register target: small struct initialization */
    register struct PackedStruct reg_target = { .a = 1, .b = 2, .c = 3 };
    /* Use volatile to prevent optimization */
    volatile int dummy = reg_target.a + reg_target.b;
    (void)dummy;
}

void test_automatic_variables(void) {
    /* Automatic array with constant range - might be considered for register */
    int auto_array[6] = { [L ... H] = 77 };  /* L=2, H=5, count=4 > 2 */
    
    /* Volatile ensures MEM_P(target)=true */
    volatile int vol_array[8] = { [1 ... 3] = 888 };
    
    /* Use them to prevent dead code elimination */
    printf("auto_array[2]=%d, vol_array[1]=%d\n", auto_array[2], vol_array[1]);
}

void test_conditional_init(int selector) {
    /* Initialization inside conditional with constant condition */
    if (selector > 0) {
        /* This should still be parsed as constant bounds */
        int cond_array[12] = { [5 ... 9] = 555 };
        printf("cond_array[5]=%d\n", cond_array[5]);
    }
}

/* ==================== 6. Multi-Dimensional and Nested Aggregates ==================== */

void test_multi_dimensional(void) {
    /* Multi-dimensional array with constant range */
    int md[3][4] = { [0 ... 1][2 ... 3] = 5 };
    
    /* Nested struct with array initialization */
    struct Container cont = { 
        .id = 1001,
        .data = { [1 ... 3] = 7 }  /* count = 3 > 2 */
    };
    
    printf("md[0][2]=%d, md[1][3]=%d\n", md[0][2], md[1][3]);
    printf("cont.data[2]=%d\n", cont.data[2]);
}

/* ==================== Main Function ==================== */

int main(void) {
    printf("Testing constant bounds initialization paths...\n");
    
    /* 1. Register target scenario (hopefully !MEM_P(target)) */
    test_register_target();
    
    /* 2. Static initializations (MEM_P(target)=true) */
    printf("small_range_array[0]=%d\n", small_range_array[0]);
    printf("large_array[50]=%d\n", large_array[50]);
    printf("aligned_array[5]=%d\n", aligned_array[5]);
    
    /* 3. Automatic variables with different counts */
    test_automatic_variables();
    
    /* 4. Conditional initialization */
    test_conditional_init(1);
    
    /* 5. Multi-dimensional and nested aggregates */
    test_multi_dimensional();
    
    /* Additional test: compound literal (creates initialization context) */
    struct PackedStruct *ptr = &(struct PackedStruct){ .a = 2, .b = 4, .c = 1 };
    printf("compound literal: a=%u\n", ptr->a);
    
    /* Test with exactly 1 element range (count=1) */
    int single_range[20] = { [15] = 999 };  /* count = 1 */
    printf("single_range[15]=%d\n", single_range[15]);
    
    /* Test with exactly 2 elements but non-contiguous (still count=2) */
    int two_elem[10] = { [2] = 1, [7] = 2 };  /* Two separate designators, each count=1 */
    /* Note: For designated initializers with separate indices, each is processed separately */
    
    /* Better test for count=2: contiguous range */
    int two_contig[10] = { [4 ... 5] = 33 };  /* count = 2 */
    printf("two_contig[4]=%d\n", two_contig[4]);
    
    return 0;
}
