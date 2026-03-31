/* test_hw_loops.c - Test program for hardware loop nesting analysis */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_sink = 0;
volatile int g_array[100] = {0};

/* Mark function to prevent inlining and ensure loops stay together */
__attribute__((noinline, noipa))
static void test_nested_loops(void) {
    volatile int local_sink = 0;
    
    /* 
     * Three perfectly nested loops - creates clear parent-child relationships.
     * Loop A (outer): blocks = {A_header, A_body, A_latch}
     * Loop B (middle): blocks = {B_header, B_body, B_latch} ⊆ A_body
     * Loop C (inner): blocks = {C_header, C_body, C_latch} ⊆ B_body
     */
    for (int i = 0; i < 10; ++i) {           /* Loop A - outermost */
        for (int j = 0; j < 8; ++j) {        /* Loop B - middle */
            for (int k = 0; k < 6; ++k) {    /* Loop C - innermost */
                /* Simple body to create basic blocks */
                local_sink = i * j + k;
                g_array[(i * 8 + j) * 6 + k] = local_sink;
            }
            /* Middle loop tail - part of Loop B but not Loop C */
            g_sink += j;
        }
        /* 
         * Loop D: Partially overlaps with Loop A but not perfectly nested.
         * Starts inside Loop A's body (after Loop B) but continues
         * beyond where Loop B ends. This creates:
         * - bitmap_intersect_p(A, D) = true (share some blocks)
         * - !bitmap_intersect_compl_p(D, A) = false (D has blocks outside A)
         * - !bitmap_intersect_compl_p(A, D) = false (A has blocks outside D)
         * So neither is subset of the other -> continue
         */
        for (int m = 0; m < 5; ++m) {        /* Loop D - partially overlapping */
            local_sink = i * m;
            g_array[50 + m] = local_sink;
        }
    }
    
    /* 
     * Loop E: Separate loop that doesn't intersect with any previous loops.
     * This ensures we have multiple loops to analyze but no subset relationship
     * with the nested loops above.
     */
    for (int n = 0; n < 12; ++n) {           /* Loop E - disjoint */
        g_sink -= n;
        g_array[n] = g_sink;
    }
    
    /* 
     * Additional nested structure to ensure multiple parent-child relationships
     * are established in the analysis.
     */
    for (int x = 0; x < 7; ++x) {            /* Loop F */
        for (int y = 0; y < 5; ++y) {        /* Loop G ⊆ F */
            local_sink = x * y;
            g_array[70 + x * 5 + y] = local_sink;
        }
    }
}

/* Main function with architecture-specific compilation hints */
int main(void) {
    /* Initialize to prevent constant propagation */
    for (int i = 0; i < 100; ++i) {
        g_array[i] = i;
    }
    
    /* Call the loop-intensive function */
    test_nested_loops();
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        sum += g_array[i];
    }
    
    printf("Result: %d (g_sink = %d)\n", sum, g_sink);
    return 0;
}

/* 
 * Compilation instructions for different targets:
 * 
 * 1. For RISC-V with Ziloop extension:
 *    riscv64-unknown-linux-gnu-gcc -O2 -march=rv64gc_ziloop -fhwloops \
 *      -fno-unroll-loops -funroll-all-loops -fdump-rtl-hwloops \
 *      -S test_hw_loops.c -o test_hw_loops.s
 * 
 * 2. For ARC targets:
 *    arc-elf-gcc -O2 -mloop -ffunction-sections \
 *      -fno-unroll-loops -fdump-tree-hwloops \
 *      test_hw_loops.c -o test_hw_loops.elf
 * 
 * 3. Generic (if hardware loops supported):
 *    gcc -O3 -fhwloops -fno-unroll-loops \
 *      -fdump-rtl-all -fdump-rtl-hwloops \
 *      test_hw_loops.c -o test_hw_loops
 * 
 * The loops are designed to trigger specific cases:
 * 1. Loops A, B, C: Perfect nesting -> B is child of A, C is child of B
 * 2. Loop D: Partially overlaps with A -> bitmap_intersect_p succeeds
 *    but subset checks fail -> continue path taken
 * 3. Loop E: Disjoint -> bitmap_intersect_p fails -> continue
 * 4. Loops F, G: Another perfect nesting -> establishes parent-child
 */
