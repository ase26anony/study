/* test_hw_loops.c
 * Designed to trigger hardware loop optimization bitmap intersection analysis
 * Specifically targets lines 429-436 in hw-doloop.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Global volatile variables to prevent optimization */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Force this function to stay intact for loop analysis */
__attribute__((noinline, noipa))
void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* ============================================
     * PART 1: Three perfectly nested loops
     * Creates clear parent-child relationships:
     * - Loop i (outermost) contains blocks A
     * - Loop j (middle) contains blocks A+B
     * - Loop k (innermost) contains blocks A+B+C
     * ============================================ */
    for (int i = 0; i < 10; ++i) {
        /* Block A: Outer loop header/body start */
        g_array[i] = i;  /* Simple operation */
        
        for (int j = 0; j < 8; ++j) {
            /* Block B: Middle loop header/body */
            local_sink = i * j;
            
            for (int k = 0; k < 6; ++k) {
                /* Block C: Innermost loop - proper subset of outer loops */
                g_sink = i + j + k;
                g_array[k] = local_sink + k;
            }
            /* Block D: Middle loop latch */
            g_sink += j;
        }
        /* Block E: Outer loop continues... */
        
        /* ============================================
         * PART 2: Partially overlapping loop
         * This loop shares Block E with the outer i loop
         * but is NOT a perfect subset
         * Should trigger: bitmap_intersect_p = true
         * but bitmap_intersect_compl_p also true for both
         * ============================================ */
        for (int m = 0; m < 5; ++m) {
            /* Block F: Overlapping loop body
             * Shares some blocks with outer i loop (Block E)
             * but not all - creates partial overlap */
            g_array[i + 10] = m * 2;
            local_sink = m + i;
        }
        /* Block G: Continuation of outer i loop after overlapping loop */
        g_sink += i;
    }
    
    /* ============================================
     * PART 3: Separate non-intersecting loop
     * This loop doesn't share any blocks with the nested loops
     * Should trigger: bitmap_intersect_p = false
     * ============================================ */
    for (int n = 0; n < 7; ++n) {
        /* Block H: Completely separate loop */
        g_array[n + 50] = n * 3;
        local_sink -= n;
    }
    
    /* ============================================
     * PART 4: Another nested structure at same level
     * Creates sibling loop relationship
     * ============================================ */
    for (int p = 0; p < 4; ++p) {
        for (int q = 0; q < 3; ++q) {
            g_sink = p * q;
            g_array[p * 3 + q] = g_sink;
        }
    }
}

/* Main function with side effects */
int main(void) {
    printf("Starting hardware loop test...\n");
    
    /* Initialize array */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the function with complex loop structure */
    test_nested_loops();
    
    /* Create observable side effect */
    int sum = 0;
    for (int i = 0; i < 20; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return sum > 0 ? 0 : 1;
}

/* ============================================
 * Compilation instructions for coverage:
 * 
 * For RISC-V with Ziloop extension:
 *   riscv64-unknown-elf-gcc -O2 -march=rv64gc_ziloop -fhwloops \
 *     -fno-unroll-loops -funroll-all-loops \
 *     -fdump-rtl-hwloops -fdump-rtl-all \
 *     -S test_hw_loops.c -o test_hw_loops.s
 * 
 * For ARC targets:
 *   arc-elf-gcc -O2 -mloop -ffunction-sections \
 *     -fno-unroll-loops -fdump-tree-hwloops \
 *     test_hw_loops.c -o test_hw_loops.elf
 * 
 * For generic testing (if hardware loops supported):
 *   gcc -O3 -fhwloops -fno-unroll-loops \
 *     -fdump-rtl-hwloops -fdump-tree-loop \
 *     test_hw_loops.c -o test_hw_loops
 * 
 * To verify the bitmap intersection logic is triggered,
 * examine the hwloops dump file for messages about
 * loop nesting analysis and parent-child relationships.
 * ============================================ */
