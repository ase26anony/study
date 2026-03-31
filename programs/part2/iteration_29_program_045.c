/* test_hw_loops.c
 * This program creates specific loop nesting patterns to trigger
 * hardware loop optimization analysis, particularly the bitmap
 * intersection logic in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization and ensure loops remain distinct */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure all loops are
 * analyzed together in the same context */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * ============================================ */
    for (int i = 0; i < 5; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 4; ++j) {      /* Middle loop L2 */
            for (int k = 0; k < 3; ++k) {  /* Inner loop L3 */
                /* Simple body - creates basic blocks */
                local_sink = i * j + k;
                g_array[(i * 16 + j * 4 + k) % 100] = local_sink;
            }
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * Shares blocks with L1 but not perfectly nested
         * This triggers bitmap_intersect_p() but not subset checks
         * ============================================ */
        for (int m = 0; m < 3; ++m) {      /* Loop L4 - overlaps with L1 */
            /* This loop starts in L1's body but continues
             * after L2/L3, creating partial overlap */
            local_sink = i * m + 7;
            g_array[(i * 10 + m) % 100] = local_sink;
            
            /* Small conditional to create additional basic blocks
             * within L4, ensuring it's not a subset of L1 */
            if (m % 2 == 0) {
                g_sink += 1;
            } else {
                g_sink -= 1;
            }
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * Provides variety for the analysis
     * ============================================ */
    for (int n = 0; n < 8; ++n) {          /* Loop L5 - separate, no overlap */
        local_sink = n * 2;
        g_array[n % 100] = local_sink;
    }
    
    /* Additional loop to create more relationships */
    for (int p = 0; p < 6; ++p) {          /* Loop L6 */
        /* Another level of nesting with different structure */
        for (int q = 0; q < p + 1; ++q) {  /* Loop L7 - size depends on p */
            local_sink = p * q;
            g_array[(p + q) % 100] = local_sink;
        }
    }
    
    /* Final operation to prevent complete optimization */
    g_sink = local_sink;
}

/* Main function with compilation target guards */
int main(void) {
#ifdef __riscv__
    printf("Compiled for RISC-V with hardware loop support\n");
#elif defined(__ARC__)
    printf("Compiled for ARC with hardware loop support\n");
#elif defined(__DSP__) || defined(__dsPIC__)
    printf("Compiled for DSP target with hardware loop support\n");
#else
    printf("Compiled for generic target (may not trigger hw-doloop)\n");
#endif
    
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the function with complex loop nesting */
    test_nested_loops();
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (array[0]=%d)\n", g_sink, g_array[0]);
    
    return 0;
}
