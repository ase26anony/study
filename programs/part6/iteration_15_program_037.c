/* test_hwloop.c
 * This program is designed to trigger the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) that validates the canonical decrementing
 * loop pattern: (compare (plus reg -1) 0).
 *
 * Compile with: gcc -O2 -fdump-rtl-loop2 -fno-unroll-loops -c test_hwloop.c
 * Or for ARM hardware loop testing: gcc -O2 -march=armv7-a -mthumb -fdump-rtl-doloop -c test_hwloop.c
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from
 * early optimizations and ensure they reach the hardware loop detection pass.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;  /* Prevent elimination of loop bodies */
    
    /* Loop 1: Canonical decrementing loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from Loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with > 0 condition, different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinguishing constant */
    }
    
    /* Use sum to prevent dead code elimination */
    if (sum > 1000) {
        printf("Unexpected large sum\n");  /* This should never happen */
    }
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function call isn't optimized away */
    printf("Test completed (loops executed)\n");
    
    return 0;
}
