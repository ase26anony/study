/* test_hw_loops.c
 * Designed to trigger hardware loop optimization bitmap intersection analysis
 * Specifically targets lines 429-436 in hw-doloop.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function as noinline to ensure loops are analyzed together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * PART 1: Three perfectly nested countable loops
     * This creates clear subset relationships:
     * - k-loop blocks ⊆ j-loop blocks ⊆ i-loop blocks
     */
    for (int i = 0; i < 10; ++i) {          /* Outer loop L1 */
        for (int j = 0; j < 8; ++j) {       /* Middle loop L2 */
            for (int k = 0; k < 6; ++k) {   /* Inner loop L3 */
                /* Simple body - prevents optimization but keeps basic blocks simple */
                local_sink = i * j + k;
                g_array[k] = local_sink;    /* Use array to create memory ops */
            }
            /* This creates a basic block between inner and middle loops */
            g_sink += j;
        }
        
        /*
         * PART 2: Non-nested, intersecting loop
         * This loop shares the i-loop's header block but has different body blocks
         * It starts inside i-loop's body but continues differently
         * This should trigger: bitmap_intersect_p = true, but subset checks fail
         */
        for (int m = 0; m < 5; ++m) {       /* Intersecting loop L4 */
            /* Different body from the nested loops above */
            local_sink = i * m * 2;
            g_array[m + 10] = local_sink;
        }
        
        /* Another basic block in i-loop after L4 */
        g_sink -= i;
    }
    
    /*
     * PART 3: Separate loop that doesn't intersect with the nested structure
     * This provides variety for the analysis
     */
    for (int n = 0; n < 7; ++n) {           /* Disjoint loop L5 */
        local_sink = n * 3;
        g_array[n + 20] = local_sink;
    }
    
    /*
     * PART 4: Additional complexity - loop with early exit
     * Creates more interesting control flow
     */
    for (int p = 0; p < 12; ++p) {          /* Loop L6 */
        if (p > 8) {
            /* Creates separate exit path */
            break;
        }
        local_sink = p * p;
        g_array[p + 30] = local_sink;
    }
}

/* Alternative test with more explicit overlapping */
__attribute__((noinline, noipa))
static void test_overlapping_loops(void) {
    volatile int sink = 0;
    int arr[50] = {0};
    
    /* Loop A */
    for (int a = 0; a < 15; ++a) {
        arr[a] = a;
        
        /* Loop B - nested inside A */
        for (int b = 0; b < 10; ++b) {
            sink = a * b;
            arr[b + 15] = sink;
            
            /* Loop C - deeply nested */
            for (int c = 0; c < 5; ++c) {
                sink = a * b * c;
                arr[c + 25] = sink;
            }
        }
        
        /* Loop D - starts in A's body but has different continuation
         * Shares some blocks with A but not a perfect subset
         */
        for (int d = a; d < 10; ++d) {
            sink = a + d;
            arr[d + 30] = sink;
            /* This creates partial overlap - some blocks in A, some not */
            if (d > 5) {
                sink *= 2;
            }
        }
    }
    
    /* Loop E - separate, non-intersecting */
    for (int e = 0; e < 20; ++e) {
        arr[e + 40] = e * 3;
    }
}

/* Main function with architecture-specific compilation */
int main(void) {
    printf("Testing hardware loop optimization patterns\n");
    
    /* Call test functions */
    test_nested_loops();
    test_overlapping_loops();
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", g_sink + g_array[0]);
    
    return 0;
}

/* 
 * Compilation instructions for different targets:
 * 
 * 1. For RISC-V with Ziloop extension:
 *    riscv64-unknown-elf-gcc -O2 -march=rv64gc_ziloop -fhwloops \
 *      -fno-unroll-loops -funroll-all-loops \
 *      -fdump-rtl-hwloops -fdump-rtl-all \
 *      -S test_hw_loops.c -o test_hw_loops.s
 * 
 * 2. For ARC targets:
 *    arc-elf-gcc -O2 -mloop -ffunction-sections \
 *      -fno-unroll-loops -fdump-tree-hwloops \
 *      test_hw_loops.c -o test_hw_loops.elf
 * 
 * 3. Generic (if hardware loops supported):
 *    gcc -O3 -fhwloops -fno-unroll-loops \
 *      -fdump-rtl-hwloops -fdump-tree-all \
 *      test_hw_loops.c -o test_hw_loops
 * 
 * 4. To see the optimization in action:
 *    Add -da flag to see all RTL dumps and look for hw-doloop pass
 */
