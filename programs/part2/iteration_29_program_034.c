/* test_hw_loops.c - Target hardware loop optimization coverage */

/* Prevent unwanted inlining and transformations */
#define NOINLINE __attribute__((noinline, noipa))

/* Use volatile to prevent loop elimination */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Main test function with carefully structured loops */
NOINLINE void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear subset relationships:
     * Loop k's blocks ⊆ Loop j's blocks ⊆ Loop i's blocks
     */
    for (int i = 0; i < 10; ++i) {
        /* Basic block A - start of outer loop */
        local_sink += i;
        
        for (int j = 0; j < 8; ++j) {
            /* Basic block B - start of middle loop */
            local_sink -= j;
            
            for (int k = 0; k < 6; ++k) {
                /* Basic block C - innermost loop body */
                /* Simple operation that can't be optimized away */
                g_array[k] = i * j + k;
                local_sink += g_array[k];
            }
            /* Basic block D - end of middle loop */
            local_sink *= 2;
        }
        /* Basic block E - still in outer loop, after middle loop */
        
        /*
         * PARTIALLY OVERLAPPING LOOP - triggers bitmap_intersect_p = true
         * but subset checks fail (neither is subset of the other)
         * 
         * This loop shares basic blocks with the outer i loop:
         * - Starts in block E (same as outer loop)
         * - Has its own unique blocks for the loop body
         * - Ends before outer loop ends
         */
        for (int m = 0; m < 5; ++m) {
            /* Basic block F - unique to this loop */
            g_array[m + 10] = i * m;
            local_sink -= g_array[m + 10];
        }
        /* Basic block G - back to outer loop flow */
        
        /* Another inner loop - creates another child relationship */
        for (int n = 0; n < 3; ++n) {
            /* Basic block H - another perfectly nested loop */
            g_array[n + 20] = i * n;
            local_sink += g_array[n + 20];
        }
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP - should not intersect with any above
     * This ensures bitmap_intersect_p = false for some comparisons
     */
    for (int p = 0; p < 7; ++p) {
        /* Basic block I - completely separate */
        g_sink += p * 2;
        g_array[p + 30] = g_sink;
    }
    
    /* Final operation to use the result */
    g_sink = local_sink;
}

/* 
 * Additional function with different nesting pattern
 * to ensure multiple loop structures are analyzed
 */
NOINLINE void test_more_loops(void) {
    volatile int sink2 = 0;
    
    /* Double nested loop */
    for (int x = 0; x < 5; ++x) {
        for (int y = 0; y < 4; ++y) {
            sink2 += x * y;
            g_array[x * 4 + y] = sink2;
        }
        
        /* Loop that shares some blocks but not all */
        for (int z = 0; z < 3; ++z) {
            sink2 -= x * z;
        }
    }
    
    g_sink += sink2;
}

/* Main function */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call test functions multiple times to ensure they're not optimized away */
    for (int iter = 0; iter < 3; ++iter) {
        test_nested_loops();
        test_more_loops();
    }
    
    /* Use results to prevent dead code elimination */
    volatile int result = 0;
    for (int i = 0; i < 50; ++i) {
        result += g_array[i];
    }
    
    /* Print to create observable side effect */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}
