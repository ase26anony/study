/* test_hwloops.c - Test program for hardware loop optimization bitmap analysis */

/* Prevent optimizations that might interfere with loop structure */
#pragma GCC optimize("no-unroll-loops")
#pragma GCC optimize("no-peel-loops")

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink = 0;
volatile int arr[100] = {0};

/* Function containing all loop structures - marked noinline to ensure analysis */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * Outer loop (i) contains middle loop (j)
     * Middle loop (j) contains inner loop (k)
     * ============================================ */
    for (int i = 0; i < 10; ++i) {
        /* Basic block A - start of outer loop body */
        sink = i * 2;
        arr[i] = sink;
        
        for (int j = 0; j < 8; ++j) {
            /* Basic block B - start of middle loop body */
            sink = j * 3;
            
            for (int k = 0; k < 6; ++k) {
                /* Basic block C - innermost loop body */
                sink = i * j * k;
                global_sink += sink;
            }
            
            /* Basic block D - after inner loop, still in middle loop */
            arr[j] += sink;
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares basic blocks with the outer loop
         * but is NOT perfectly nested within it
         * Should trigger bitmap_intersect_p() = true
         * but subset checks fail
         * ============================================ */
        for (int m = 0; m < 5; ++m) {
            /* Basic block E - partially overlapping loop body
             * This block is inside outer loop's body but
             * the loop structure is different */
            sink = i * m * 7;
            arr[m + 10] = sink;
            
            /* This creates partial overlap - the loop continues
             * after the middle loop ends but before outer loop ends */
        }
        
        /* Basic block F - still in outer loop, after partial loop */
        arr[i] *= 2;
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * This loop doesn't share blocks with any other
     * Should trigger bitmap_intersect_p() = false
     * ============================================ */
    for (int x = 0; x < 12; ++x) {
        /* Basic block G - completely separate loop */
        sink = x * x;
        global_sink -= sink;
    }
    
    /* ============================================
     * PART 4: Another nested structure with different bounds
     * Creates additional parent-child relationships
     * ============================================ */
    for (int a = 0; a < 7; ++a) {
        /* Basic block H */
        sink = a * 11;
        
        for (int b = 0; b < 5; ++b) {
            /* Basic block I - inner to H */
            sink = a + b;
            arr[b + 20] = sink;
            
            /* Small conditional to create multiple basic blocks
             * within the loop body */
            if (sink > 3) {
                /* Basic block J - conditional true path */
                global_sink += 1;
            } else {
                /* Basic block K - conditional false path */
                global_sink -= 1;
            }
        }
        
        /* Basic block L - after inner loop */
        arr[a] = sink;
    }
}

/* Main function to call the test and ensure side effects */
int main(void) {
    /* Initialize array with some values */
    for (int i = 0; i < 100; ++i) {
        arr[i] = i;
    }
    
    /* Call the function with all loop structures */
    test_nested_loops();
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += arr[i];
    }
    
    /* Print something to ensure execution */
    printf("Result: %d (global_sink: %d)\n", sum, global_sink);
    
    return sum > 0 ? 0 : 1;
}
