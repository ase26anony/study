/* test_hw_loops.c - Test program for hardware loop nesting analysis */
#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100];

/* Function containing nested and intersecting loops */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body - L3 blocks are subset of L2, which are subset of L1 */
                local_sink = i * j + k;
                g_array[(i * 8 + j) % 100] = local_sink;
            }
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * Shares blocks with L1 but not perfectly nested
         * This triggers bitmap_intersect_p() = true
         * but subset checks fail
         * ============================================ */
        for (int m = 0; m < 5; ++m) {       /* Loop L4 - partially overlaps with L1 */
            /* This loop starts in L1's body but continues independently */
            local_sink = i * m;
            g_sink += local_sink;
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * Provides variety for the analysis
     * ============================================ */
    for (int n = 0; n < 7; ++n) {           /* Loop L5 - no intersection with others */
        local_sink = n * 2;
        g_array[n % 100] = local_sink;
    }
    
    /* Additional complexity to prevent over-simplification */
    for (int p = 0; p < 3; ++p) {           /* Loop L6 - another simple loop */
        for (int q = 0; q < 4; ++q) {       /* Loop L7 - nested inside L6 */
            g_sink += p * q;
        }
    }
}

/* Main function */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", g_sink + g_array[0]);
    
    return 0;
}
