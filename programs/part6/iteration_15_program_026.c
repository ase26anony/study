/* test_hwloop_pattern.c
 * Designed to trigger the uncovered validation block in GCC's loop-doloop.cc
 * lines 136-150: checking for (compare (plus reg -1) 0) pattern
 */

#include <stdio.h>

/* Static function with optimization attributes to preserve loop structure */
static void __attribute__((noinline, optimize("O2"))) test_loops(
    volatile int bound1, 
    volatile int bound2, 
    volatile int bound3)
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrement-to-zero pattern with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent elimination */
    }
    
    /* Loop 2: Decrement-to-zero pattern with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrement-to-zero pattern with > comparison */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Different constant to distinguish from other loops */
    }
    
    /* Use sum to prevent dead code elimination */
    if (sum > 1000) {
        printf("Unexpected large sum\n");
    }
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to ensure execution */
    printf("Loop test completed\n");
    
    return 0;
}
