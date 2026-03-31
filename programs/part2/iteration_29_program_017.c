/* test_hw_loops.c
 * Program designed to trigger hardware loop optimization analysis
 * for nested loop bitmap intersection checks in hw-doloop.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent compiler from optimizing away loops */
volatile int sink;
volatile int result[100];

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    int i, j, k;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear subset relationships:
     * - k-loop blocks ⊆ j-loop blocks ⊆ i-loop blocks
     * ============================================ */
    for (i = 0; i < 10; ++i) {           /* Outer loop - largest block set */
        for (j = 0; j < 8; ++j) {        /* Middle loop - subset of i-loop */
            for (k = 0; k < 6; ++k) {    /* Inner loop - subset of j-loop */
                /* Simple operation to prevent loop removal */
                sink = i * j + k;
                result[k] = sink;
            }
            /* Additional basic block in j-loop but not in k-loop */
            sink = j * 2;
        }
        /* Additional basic block in i-loop but not in j-loop */
        sink = i * 3;
        
        /* ============================================
         * PART 2: Non-nested, intersecting loop
         * Shares blocks with i-loop but is not perfectly nested
         * This triggers bitmap_intersect_p() = true
         * but bitmap_intersect_compl_p() checks fail
         * ============================================ */
        for (int m = 0; m < 5; ++m) {
            /* This loop starts inside i-loop body but continues
             * after j-loop ends. It shares the i-loop header
             * but has different body blocks */
            sink = i * m + 100;
            result[m + 10] = sink;
        }
    }
    
    /* ============================================
     * PART 3: Separate loop that doesn't intersect
     * Provides variety for the analysis
     * ============================================ */
    for (int x = 0; x < 7; ++x) {
        sink = x * x;
        result[x + 20] = sink;
    }
    
    /* ============================================
     * PART 4: Another nested structure with different bounds
     * Ensures multiple loop candidates for analysis
     * ============================================ */
    for (int a = 0; a < 5; ++a) {
        for (int b = 0; b < 4; ++b) {
            sink = a + b;
            result[a * 4 + b] = sink;
        }
    }
}

/* Main function with side effects to prevent DCE */
int main(void) {
    /* Initialize result array */
    for (int i = 0; i < 100; ++i) {
        result[i] = 0;
    }
    
    /* Call the function with all loops */
    test_nested_loops();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 30; ++i) {
        sum += result[i];
    }
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
