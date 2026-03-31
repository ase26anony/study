/* test_hwloops.c - Test program for hardware loop nesting analysis */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops (i, j, k).
     * This creates clear subset relationships:
     * - k-loop blocks ⊆ j-loop blocks ⊆ i-loop blocks
     */
    for (int i = 0; i < 10; ++i) {
        /* Basic block A - start of i-loop body */
        g_array[i] = i;
        
        for (int j = 0; j < 8; ++j) {
            /* Basic block B - start of j-loop body */
            local_sink = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Basic block C - innermost loop body */
                g_sink = i + j + k;
                g_array[k] = local_sink + k;
            }
            
            /* Basic block D - after k-loop, still in j-loop */
            local_sink += j;
        }
        
        /* 
         * PARTIALLY OVERLAPPING LOOP - shares some blocks with i-loop
         * but not perfectly nested. This triggers:
         * 1. bitmap_intersect_p() returns true (shares block E)
         * 2. bitmap_intersect_compl_p() returns true (not a subset)
         * 3. Continue to next candidate
         */
        for (int m = 0; m < 5; ++m) {
            /* Basic block E - in both i-loop and m-loop */
            g_sink = i * m;
            
            /* Basic block F - only in m-loop, not in original i-loop body */
            if (m % 2 == 0) {
                g_array[m + 10] = m;
            }
        }
        /* Basic block G - back in i-loop only */
        local_sink -= i;
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP
     * This loop doesn't share any basic blocks with the nested loops above.
     * Should fail bitmap_intersect_p() check immediately.
     */
    for (int x = 0; x < 7; ++x) {
        /* Basic block H - completely separate */
        g_sink = x * 2;
        g_array[x + 20] = x;
    }
    
    /* 
     * ANOTHER NESTING LEVEL for more complex hierarchy
     * Creates: p-loop (parent) with two child loops (q and r)
     * where q and r are siblings at same nesting level
     */
    for (int p = 0; p < 12; ++p) {
        /* Basic block I - p-loop body start */
        local_sink = p;
        
        /* First child loop */
        for (int q = 0; q < 4; ++q) {
            /* Basic block J - q-loop body */
            g_sink = p * q;
            g_array[q] = p + q;
        }
        
        /* Some code between sibling loops */
        local_sink += p;
        
        /* Second child loop (sibling to q-loop) */
        for (int r = 0; r < 3; ++r) {
            /* Basic block K - r-loop body */
            g_sink = p - r;
            g_array[r + 30] = r;
        }
        
        /* Basic block L - p-loop body end */
        local_sink *= 2;
    }
}

int main(void) {
    printf("Testing hardware loop nesting analysis...\n");
    
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i % 37;
    }
    
    /* Call the function with all loop structures */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 50; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}
