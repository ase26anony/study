/* test_hardware_loops.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by creating canonical decrementing
 * loop patterns that match the RTL pattern (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from
 * early optimizations while ensuring loop optimization passes run on it.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Expected to generate: (compare (plus i -1) 0)
     */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition.
     * Also should generate the same canonical pattern.
     */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }
    
    /* Loop 3: Another decrementing loop with different variable.
     * Increases chances of pattern matching.
     */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile("" : : "r"(sum) : "memory");
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to prevent optimization of entire program */
    printf("Loop test completed (compile with -O2 -fno-unroll-loops for best results)\n");
    
    return 0;
}
