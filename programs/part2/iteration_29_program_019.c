/* test_hw_loops.c
 * This program creates specific loop nesting patterns to trigger
 * hardware loop optimization analysis, particularly the bitmap
 * intersection checks in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Function with carefully structured loops - marked noinline to ensure
 * all loops are analyzed together in the same context */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * This creates clear subset relationships:
     * - Loop k's blocks ⊆ Loop j's blocks ⊆ Loop i's blocks
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Outer loop - will be parent */
        for (int j = 0; j < 8; ++j) {       /* Middle loop - child of i */
            for (int k = 0; k < 6; ++k) {   /* Inner loop - child of j */
                /* Simple body to create basic blocks */
                local_sink = i * j + k;
                g_array[k] = local_sink;
            }
            /* Some code between inner and middle loops */
            g_sink += j;
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares blocks with loop i but is NOT
         * a perfect subset. It starts inside loop i's body
         * but continues differently.
         * 
         * Control flow:
         * - Enters when i > 5 (so shares entry with loop i)
         * - Has different exit block
         * - This triggers bitmap_intersect_p() = true
         * - But NOT bitmap_intersect_compl_p() = true
         * ============================================ */
        if (i > 5) {
            for (int m = 0; m < 4; ++m) {   /* Partially overlapping loop */
                local_sink = i * m;
                g_array[m + 10] = local_sink;
            }
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * This loop doesn't share any blocks with the
     * nested loops above.
     * ============================================ */
    for (int n = 0; n < 5; ++n) {
        local_sink = n * 2;
        g_array[n + 20] = local_sink;
    }
    
    /* ============================================
     * PART 4: Another set of nested loops with
     * different structure to provide more
     * intersection cases
     * ============================================ */
    for (int a = 0; a < 7; ++a) {
        /* First inner loop - proper subset */
        for (int b = 0; b < 5; ++b) {
            g_sink += a * b;
        }
        
        /* Second inner loop at same nesting level as first
         * Shares parent but not blocks with first inner loop */
        for (int c = 0; c < 3; ++c) {
            g_sink -= a + c;
        }
    }
}

/* Main function with compilation hints */
int main(void) {
    printf("Testing hardware loop optimization patterns\n");
    
    /* Call our test function multiple times to ensure
     * it's not optimized away */
    for (int iter = 0; iter < 3; ++iter) {
        test_nested_loops();
        
        /* Use the results to prevent dead code elimination */
        printf("Iteration %d: g_sink = %d, g_array[0] = %d\n", 
               iter, g_sink, g_array[0]);
    }
    
    return 0;
}
