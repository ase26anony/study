/* test_hw_loops.c
 * Designed to trigger hardware loop optimization bitmap intersection logic
 * Compile with: -O2 -fhwloops -fno-unroll-loops
 * For RISC-V: -march=rv64gc_ziloop
 * For ARC: -mloop
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships:
     * Loop k ⊆ Loop j ⊆ Loop i
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Outer loop - will be parent of j */
        for (int j = 0; j < 8; ++j) {       /* Middle loop - parent of k, child of i */
            for (int k = 0; k < 6; ++k) {   /* Innermost loop - child of j */
                /* Simple body to create basic blocks */
                local_sink = i * j + k;
                g_array[k] = local_sink;    /* Prevent dead code elimination */
            }
            /* Additional operation in j-loop body but outside k-loop */
            g_sink += j;
        }
        /* Additional operation in i-loop body but outside j-loop */
        g_sink ^= i;
    }
    
    /* ============================================
     * PART 2: Partially overlapping loop
     * Shares some blocks with i-loop but not perfectly nested
     * Should trigger: bitmap_intersect_p = true
     * but bitmap_intersect_compl_p = true (both ways)
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Uses same induction variable name */
        /* This loop starts in i-loop's body but has different structure */
        for (int m = 0; m < 5; ++m) {       /* Different inner loop */
            local_sink = i * m;
            g_array[m + 10] = local_sink;
        }
        /* Different control flow - no j or k loops here */
        if (i % 2 == 0) {
            g_sink += 1;
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * Provides variety in loop structure
     * ============================================ */
    for (int x = 0; x < 7; ++x) {
        g_array[x + 20] = x * x;
    }
    
    /* Final operation to ensure function has side effect */
    g_sink = local_sink;
}

/* Alternative version with more explicit block overlap */
__attribute__((noinline, noipa))
static void test_complex_overlap(void) {
    volatile int tmp = 0;
    
    /* Loop A: Outer loop with complex body */
    for (int a = 0; a < 5; ++a) {
        /* Block 1: Pre-inner loop operations */
        tmp = a * 2;
        g_array[0] = tmp;
        
        /* Loop B: Nested inside A */
        for (int b = 0; b < 4; ++b) {
            g_array[b + 1] = a + b;
        }
        
        /* Block 2: Post-inner loop operations */
        tmp += 1;
        g_array[5] = tmp;
        
        /* Loop C: Starts in A but continues after B ends
         * This creates partial overlap with A's blocks
         * C's blocks are: {Block 1?, Loop C blocks, Block 2?}
         * Actually C is completely inside A but not nested with B
         */
        for (int c = 0; c < 3; ++c) {
            g_array[c + 6] = a * c;
        }
        
        /* Block 3: Final operations in A */
        g_sink ^= a;
    }
    
    /* Loop D: Separate loop that reuses induction variable 'a'
     * but has different structure - creates intersection check
     */
    for (int a = 0; a < 5; ++a) {
        /* Different loop body structure */
        if (a % 2) {
            for (int d = 0; d < 2; ++d) {
                g_array[d + 10] = a + d;
            }
        } else {
            g_array[12] = a * 3;
        }
    }
}

int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the test functions */
    test_nested_loops();
    test_complex_overlap();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 30; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (sink: %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}
