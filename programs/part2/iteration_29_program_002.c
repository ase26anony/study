/* test_hw_loops.c - Test program for hardware loop nesting analysis */
#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Function with nested and intersecting loops - marked noinline/noipa */
#ifdef __GNUC__
__attribute__((noinline, noipa))
#endif
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body - prevents optimization */
                local_sink = i * j + k;
                g_array[(i * 8 + j) % 100] += local_sink;
            }
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * Shares blocks with L1 but not perfectly nested
         * Will trigger bitmap_intersect_p but not subset
         * ============================================ */
        for (int m = 0; m < 5; ++m) {       /* Loop L4 - partially overlaps L1 */
            /* This loop starts in L1's body but continues 
               after L2/L3, creating partial overlap */
            local_sink = i * m;
            g_array[(i * 5 + m) % 100] = local_sink;
            
            /* Small inner loop inside L4 but not related to L2/L3 */
            for (int n = 0; n < 3; ++n) {   /* Loop L5 - child of L4 */
                g_sink += n;
            }
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * Provides variety for analysis
     * ============================================ */
    for (int p = 0; p < 7; ++p) {           /* Loop L6 - no intersection */
        local_sink = p * 2;
        g_array[p % 100] = local_sink;
    }
    
    /* Final side effect */
    g_sink = local_sink;
}

/* Main function */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the test function */
    test_nested_loops();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", g_sink);
    printf("Array[0]: %d\n", g_array[0]);
    
    return 0;
}
