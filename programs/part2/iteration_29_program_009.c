/* test_hw_loops.c - Test program for hardware loop nesting analysis */
#include <stdio.h>
#include <stdint.h>

/* Prevent inlining to ensure loops stay in same function context */
__attribute__((noinline, noipa))
static void test_nested_loops(volatile int* sink) {
    volatile int local_sink = 0;
    
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body - creates basic blocks */
                local_sink = i * j * k;
                *sink += local_sink;
            }
        }
        
        /* Partially overlapping loop - shares some blocks with L1 but not perfectly nested */
        /* This starts in L1's body but continues differently */
        for (int m = 0; m < 5; ++m) {       /* Partially overlapping loop L4 */
            if (i > 3) {                    /* Creates conditional basic block inside L1 */
                local_sink = i * m;
                *sink += local_sink;
            } else {
                local_sink = m + 1;
                *sink += local_sink;
            }
        }
    }
    
    /* Separate non-intersecting loop - should not intersect with any above */
    for (int n = 0; n < 7; ++n) {           /* Isolated loop L5 */
        local_sink = n * 2;
        *sink += local_sink;
    }
    
    /* Additional complexity: loop with early exit that still creates natural loop */
    for (int p = 0; p < 12; ++p) {          /* Loop L6 with internal condition */
        if (p > 8) break;                   /* Creates exit block but loop is still natural */
        local_sink = p * 3;
        *sink += local_sink;
        
        /* Small inner loop within L6 */
        for (int q = 0; q < 3; ++q) {       /* Inner loop L7 */
            local_sink = p * q;
            *sink += local_sink;
        }
    }
}

/* Main function with volatile to prevent optimization */
int main(void) {
    volatile int result = 0;
    volatile int* presult = &result;
    
    /* Call the test function multiple times to ensure it's not optimized away */
    for (int iter = 0; iter < 3; ++iter) {
        test_nested_loops(presult);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
