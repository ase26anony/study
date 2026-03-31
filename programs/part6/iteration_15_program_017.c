/* test_hardware_loops.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrementing pattern: (compare (plus reg -1) 0).
 * The loops use volatile bounds to prevent constant propagation and
 * are placed in a noinline function to ensure they reach the hardware
 * loop detection pass.
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops
 * from early optimizations while ensuring loop passes run on it.
 */
static void __attribute__((noinline, optimize("O2")))
test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;  /* Prevent dead code elimination */
    
    /* Loop 1: Canonical decrementing pattern with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }
    
    /* Loop 2: Alternative != 0 condition, same decrement pattern */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from Loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum to prevent elimination - volatile ensures side effect */
    (void)sum;
}

int main(void)
{
    /* Volatile bounds prevent constant folding/unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure execution */
    printf("Loops completed (coverage test)\n");
    
    return 0;
}
