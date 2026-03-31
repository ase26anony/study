/* test_hw_loops.c
 * This program creates specific loop nesting patterns to trigger
 * hardware loop optimization analysis, particularly the bitmap
 * intersection checks in hw-doloop.cc lines 429-436.
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Function with carefully structured loops - marked noinline to ensure
 * all loops are analyzed together in the same context */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships
     * ============================================ */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body - creates basic blocks */
                local_sink = i * j * k;
                g_array[(i * 8 + j) % 100] += k;
            }
        }
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares blocks with L1 but is not
         * perfectly nested within it
         * ============================================ */
        for (int m = 0; m < 5; ++m) {       /* Loop L4 - partially overlaps with L1 */
            /* This loop starts in L1's body but continues
             * after L2/L3, creating partial overlap */
            local_sink = i * m;
            g_array[(i * 5 + m) % 100] += 1;
        }
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * This loop doesn't intersect with any other
     * ============================================ */
    for (int n = 0; n < 7; ++n) {           /* Loop L5 - separate, no intersection */
        local_sink = n * 2;
        g_array[n % 100] += n;
    }
    
    /* ============================================
     * PART 4: Another partially overlapping structure
     * Creates more complex intersection patterns
     * ============================================ */
    for (int x = 0; x < 12; ++x) {          /* Loop L6 */
        /* First inner loop */
        for (int y = 0; y < 4; ++y) {       /* Loop L7 */
            local_sink = x + y;
        }
        
        /* This loop shares some blocks with L6 but
         * has different control flow */
        if (x < 8) {
            for (int z = 0; z < 3; ++z) {   /* Loop L8 - conditionally nested */
                local_sink = x * z;
            }
        }
    }
    
    g_sink = local_sink;
}

/* Main function with architecture-specific compilation hints */
int main(void) {
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the loop test function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (sink: %d)\n", sum, g_sink);
    return 0;
}

/* ============================================
 * Architecture-specific compilation guidance
 * ============================================ */

/* For RISC-V with Ziloop extension:
 *   riscv64-unknown-elf-gcc -O2 -march=rv64gc_ziloop -fhwloops \
 *     -fno-unroll-loops -funroll-all-loops \
 *     -fdump-rtl-hwloops -fdump-rtl-doloop \
 *     -S test_hw_loops.c -o test_hw_loops.s
 *
 * For ARC targets:
 *   arc-elf-gcc -O2 -mloop -ffunction-sections \
 *     -fno-unroll-loops -fdump-tree-hwloops \
 *     test_hw_loops.c -o test_hw_loops.elf
 *
 * For generic testing (if hardware loops supported):
 *   gcc -O3 -fhwloops -fno-unroll-loops \
 *     -fdump-rtl-all -fdump-tree-all \
 *     test_hw_loops.c -o test_hw_loops
 */
