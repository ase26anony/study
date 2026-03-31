/* test_hwloops.c - Target hardware loop bitmap intersection analysis */

/* Prevent optimizations that might interfere with loop structure */
#pragma GCC optimize("O2")
#pragma GCC optimize("no-unroll-loops")

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent dead code elimination */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships.
     * Loop A (outer): blocks = {A_header, A_body, A_latch}
     * Loop B (middle): blocks = {B_header, B_body, B_latch} ⊆ A_body
     * Loop C (inner): blocks = {C_header, C_body, C_latch} ⊆ B_body
     */
    
    /* Loop A - outermost */
    for (int i = 0; i < 10; ++i) {
        /* Loop B - middle */
        for (int j = 0; j < 8; ++j) {
            /* Loop C - innermost */
            for (int k = 0; k < 6; ++k) {
                /* Simple body to create basic blocks */
                local_sink = i * j * k;
                g_array[(i * 8 + j) * 6 + k] = local_sink;
            }
            /* Middle loop body continuation */
            g_sink += j;
        }
        
        /*
         * Loop D - Partially overlapping with Loop A
         * Shares some blocks with Loop A but not perfectly nested.
         * Starts inside Loop A's body but continues differently.
         * This triggers: bitmap_intersect_p = true
         * but bitmap_intersect_compl_p both ways = true
         * causing the 'continue' path to be taken.
         */
        for (int d = 0; d < 5; ++d) {
            /* Uses i from outer loop, creating intersection */
            local_sink = i * d;
            g_array[d] = local_sink;
            
            /* Additional operation to create distinct basic blocks */
            if (d % 2 == 0) {
                g_sink += 1;
            }
        }
    }
    
    /*
     * Loop E - Separate, non-intersecting loop
     * Has no block intersection with any other loop.
     * This ensures bitmap_intersect_p = false for all comparisons.
     */
    for (int e = 0; e < 7; ++e) {
        local_sink = e * 2;
        g_array[e + 50] = local_sink;
    }
    
    /* Additional loop structure to ensure multiple candidates */
    for (int x = 0; x < 3; ++x) {
        for (int y = 0; y < 4; ++y) {
            local_sink = x + y;
            g_array[x * 4 + y + 70] = local_sink;
        }
    }
}

/* Alternative implementation with explicit control flow for partial overlap */
__attribute__((noinline, noipa))
void test_partial_overlap(void) {
    volatile int sink = 0;
    
    /* Create a more explicit partial overlap scenario */
    for (int outer = 0; outer < 5; ++outer) {
        /* First inner loop - perfectly nested */
        for (int inner1 = 0; inner1 < 3; ++inner1) {
            sink = outer * inner1;
        }
        
        /*
         * Second loop that partially overlaps:
         * - Shares the outer loop's header and latch
         * - But has its own body blocks that aren't in the first inner loop
         * - And doesn't contain all of outer's blocks
         */
        int temp = outer;
        while (temp < 5) {  /* while loop to create different block structure */
            sink += temp;
            temp++;
            
            /* Nested inside the partial overlap loop */
            for (int deep = 0; deep < 2; ++deep) {
                sink -= deep;
            }
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
    test_partial_overlap();
    
    /* Use results to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}
