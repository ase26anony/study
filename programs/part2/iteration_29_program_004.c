/* test_hw_loops.c - Target hardware loop optimization coverage */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    /* Three perfectly nested loops - creates clear parent-child relationships */
    for (int i = 0; i < 10; ++i) {
        /* Outer loop body start */
        g_array[i] = i;
        
        for (int j = 0; j < 8; ++j) {
            /* Middle loop body */
            int temp = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Innermost loop - simple countable operation */
                g_sink = i + j + k;
                g_array[k] = temp + k;
            }
            
            /* Middle loop continuation */
            g_sink = j * 2;
        }
        
        /* 
         * PARTIALLY OVERLAPPING LOOP 
         * This loop shares the outer loop's blocks but is not a perfect subset.
         * It starts inside the outer loop's body but continues differently.
         * This triggers bitmap_intersect_p() but not the subset checks.
         */
        for (int m = 0; m < 5; ++m) {
            /* This loop uses 'i' from outer loop but has different structure */
            g_sink = i * m + 1;
            
            /* Add some conditional to create different basic blocks */
            if (m % 2 == 0) {
                g_array[m] = i + m;
            } else {
                g_sink = m * 3;
            }
        }
        
        /* Outer loop continuation */
        g_sink = i * 10;
    }
    
    /* 
     * SEPARATE NON-INTERSECTING LOOP 
     * This loop doesn't intersect with the nested loops above.
     * Should not trigger any parent-child relationship.
     */
    for (int x = 0; x < 7; ++x) {
        g_array[x + 20] = x * x;
        for (int y = 0; y < 3; ++y) {
            g_sink = x - y;
        }
    }
    
    /* 
     * ANOTHER NESTING LEVEL VARIATION 
     * Creates more complex nesting for analysis
     */
    for (int a = 0; a < 4; ++a) {
        g_sink = a;
        
        /* Inner loop with different bounds */
        for (int b = 0; b < a + 2; ++b) {
            g_array[b] = a * b;
            
            /* Very inner loop */
            for (int c = 0; c < 2; ++c) {
                g_sink = a + b + c;
            }
        }
        
        /* Another partially overlapping loop in same scope */
        for (int d = 0; d < 3; ++d) {
            if (a > d) {
                g_sink = a - d;
            }
        }
    }
}

/* Main function with architecture detection */
int main(void) {
    printf("Testing hardware loop optimization patterns\n");
    
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the function with complex loop nesting */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 20; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (sink: %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}

/* 
 * Architecture-specific adaptations for hardware loop support
 * Uncomment the appropriate section for your target
 */

/*
// RISC-V with Ziloop extension
#ifdef __riscv__
__attribute__((target("arch=+ziloop")))
void riscv_hwloop_hint(void) {
    // Empty function to hint compiler about hardware loop support
}
#endif

// ARC processors
#ifdef __arc__
__attribute__((optimize("loop")))
void arc_hwloop_hint(void) {
    // Hint for ARC hardware loops
}
#endif

// Generic hardware loop support
#if defined(__has_feature)
#if __has_feature(hwloops)
__attribute__((optimize("hwloops")))
void generic_hwloop_hint(void) {
    // Generic hardware loop hint
}
#endif
#endif
*/
