/* test_hardware_loops.c
 * Targets GCC's loop-doloop.cc validation logic for hardware loop patterns.
 * Specifically aims to trigger the uncovered block checking for:
 *   (compare (plus reg -1) 0)
 */

#include <stdio.h>

/* Static function with optimization attributes to preserve loop structure */
static void __attribute__((noinline, optimize("O2"))) test_loops(
    volatile int bound1, 
    volatile int bound2, 
    volatile int bound3)
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent elimination */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with > 0 condition */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Different constant again */
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    if (sum > 0) {
        /* Empty - just to use sum */
    }
}

int main(void) {
    /* Volatile bounds to prevent constant propagation */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 50;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to ensure execution */
    printf("Test completed (loops executed)\n");
    
    return 0;
}
