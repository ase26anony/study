/* test_hw_loops.c - Test program for hardware loop optimization coverage */

/* Prevent optimizations that might interfere with loop analysis */
#pragma GCC optimize("no-unroll-loops")
#pragma GCC optimize("no-peel-loops")

/* Global volatile variables to prevent dead code elimination */
volatile int global_sink = 0;
volatile int results[100] = {0};

/* Function containing all loop structures - marked noinline to ensure analysis */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int sink;
    int i, j, k;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * ============================================ */
    
    /* Outer loop - will be parent of j and k loops */
    for (i = 0; i < 10; ++i) {
        /* Middle loop - child of i, parent of k */
        for (j = 0; j < 8; ++j) {
            /* Innermost loop - child of j */
            for (k = 0; k < 6; ++k) {
                /* Simple operation that can't be optimized away */
                sink = i * j + k;
                results[i * 8 + j] += sink;
            }
        }
        
        /* ============================================
         * PART 2: Non-nested, intersecting loop
         * Shares blocks with outer loop but not perfectly nested
         * This triggers bitmap_intersect_p() but not subset checks
         * ============================================ */
        
        /* This loop starts inside the i loop body but continues
           after the j loop ends, creating partial overlap */
        for (int m = 0; m < 5; ++m) {
            sink = i * m;
            results[50 + m] += sink;
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * Provides variety for the analysis
     * ============================================ */
    
    /* This loop doesn't intersect with the nested structure above */
    for (int n = 0; n < 7; ++n) {
        sink = n * 2;
        results[80 + n] = sink;
    }
    
    /* Additional loop structure to ensure multiple candidates */
    for (int p = 0; p < 4; ++p) {
        for (int q = 0; q < 3; ++q) {
            sink = p * q;
            results[90 + p] += sink;
        }
    }
}

/* Main function with architecture-specific compilation */
int main(void) {
    /* Initialize results array */
    for (int i = 0; i < 100; ++i) {
        results[i] = 0;
    }
    
    /* Call the function with all loop structures */
    test_nested_loops();
    
    /* Use the results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += results[i];
    }
    
    global_sink = sum;
    
    /* Print something to ensure execution */
    #ifdef __riscv__
    asm volatile ("nop" : : : "memory");
    #elif defined(__ARC__)
    asm volatile ("nop" : : : "memory");
    #else
    /* Generic output for other targets */
    volatile int output = global_sink;
    #endif
    
    return 0;
}
