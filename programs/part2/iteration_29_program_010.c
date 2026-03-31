/* test_hw_loops.c - Test program for hardware loop nesting analysis */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function as noinline to ensure loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear subset relationships:
     * - Loop k's blocks ⊆ Loop j's blocks ⊆ Loop i's blocks
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Outer loop - will be parent */
        for (int j = 0; j < 8; ++j) {       /* Middle loop - child of i */
            for (int k = 0; k < 6; ++k) {   /* Inner loop - child of j */
                /* Simple body to create basic blocks */
                local_sink = i * j + k;
                g_array[k] = local_sink;
            }
            /* Some code between inner loops to create distinct blocks */
            g_sink += j;
        }
        
        /* ============================================
         * PART 2: Non-nested, intersecting loop
         * This loop shares blocks with the outer i loop
         * but is NOT a subset of it (continues after j loop)
         * Will trigger: bitmap_intersect_p = true
         * but bitmap_intersect_compl_p = false for both
         * ============================================ */
        for (int m = 0; m < 5; ++m) {
            /* This loop body starts in i's scope but is not nested in j */
            local_sink = i * m;
            g_array[m + 10] = local_sink;
            
            /* Add some control flow to create more blocks */
            if (m % 2 == 0) {
                g_sink += 1;
            }
        }
    }
    
    /* ============================================
     * PART 3: Separate, non-intersecting loop
     * This loop doesn't share blocks with any other
     * Will trigger: bitmap_intersect_p = false
     * ============================================ */
    for (int n = 0; n < 7; ++n) {
        local_sink = n * 2;
        g_array[n + 20] = local_sink;
    }
    
    /* ============================================
     * PART 4: Additional nested structure
     * Another set of nested loops to ensure multiple
     * parent-child relationships are established
     * ============================================ */
    for (int a = 0; a < 5; ++a) {
        for (int b = 0; b < 4; ++b) {
            local_sink = a - b;
            g_array[b + 30] = local_sink;
        }
    }
    
    /* Final side effect */
    g_sink = local_sink;
}

/* Main function with architecture detection */
int main(void) {
    printf("Testing hardware loop nesting analysis\n");
    
    /* Architecture-specific compilation hints */
#if defined(__riscv__) || defined(__riscv)
    printf("Compiled for RISC-V with hardware loop support\n");
#elif defined(__arc__) || defined(__ARC__)
    printf("Compiled for ARC with hardware loop support\n");
#elif defined(__xtensa__)
    printf("Compiled for Xtensa with hardware loop support\n");
#else
    printf("Compiled for generic target (may need -fhwloops)\n");
#endif
    
    /* Call the test function multiple times to ensure execution */
    for (int run = 0; run < 3; ++run) {
        test_nested_loops();
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Result: %d\n", g_sink);
    printf("Array[0]=%d, Array[10]=%d, Array[20]=%d\n", 
           g_array[0], g_array[10], g_array[20]);
    
    return 0;
}
