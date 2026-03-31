/* test_hw_loops.c - Test program for hardware loop bitmap intersection analysis */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int global_sink = 0;
volatile int array[100] = {0};

/* Mark function as noinline to ensure loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * First: Create three perfectly nested loops (i, j, k)
     * This creates clear subset relationships:
     * k-loop blocks ⊆ j-loop blocks ⊆ i-loop blocks
     */
    for (int i = 0; i < 10; ++i) {
        /* Outer loop body start */
        array[i] = i * 2;
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body start */
            local_sink = i + j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop - simple operation */
                global_sink = i * j * k;
                array[k] += global_sink;
            }
            /* Middle loop body end */
            local_sink *= 2;
        }
        /* Outer loop body continues... */
        
        /*
         * Second: Create a partially overlapping loop
         * This loop shares blocks with the outer i-loop but is not
         * perfectly nested within it. It starts inside the i-loop body
         * but continues after the j-loop ends.
         * 
         * This should trigger:
         * 1. bitmap_intersect_p = true (they share blocks)
         * 2. bitmap_intersect_compl_p checks both ways = true
         * 3. Neither is subset of the other → continue
         */
        for (int m = 0; m < 5; ++m) {
            /* This loop body overlaps with i-loop but not j/k loops */
            local_sink = i * m + 1;
            array[m + 10] = local_sink;
        }
        /* Outer loop body end */
    }
    
    /*
     * Third: Create a separate loop that doesn't intersect with
     * the nested structure above
     */
    for (int n = 0; n < 12; ++n) {
        /* Completely separate loop */
        global_sink = n * 3;
        array[n + 20] = global_sink;
    }
    
    /*
     * Fourth: Create another nested structure to ensure
     * multiple parent-child relationships are established
     */
    for (int a = 0; a < 7; ++a) {
        for (int b = 0; b < 5; ++b) {
            /* Simple operation in inner loop */
            local_sink = a - b;
            array[b + 30] = local_sink;
        }
    }
}

/* Main function with compilation target guards */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        array[i] = 0;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 40; ++i) {
        sum += array[i];
    }
    
    printf("Result: %d (global_sink: %d)\n", sum, global_sink);
    return sum > 0 ? 0 : 1;
}
