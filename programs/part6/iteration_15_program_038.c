/* test_hwloop.c
 * 
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating simple decrementing
 * loops that match the canonical RTL pattern (compare (plus reg -1) 0).
 * The loops use volatile bounds to prevent constant propagation and
 * are placed in a noinline function with O2 optimization to ensure
 * they reach the hardware loop detection pass.
 */

#include <stdio.h>

/* Static function marked noinline to prevent inlining and ensure
 * optimization passes run on the isolated function body.
 * The O2 optimization level enables loop optimizations including
 * hardware loop detection.
 */
static void __attribute__((noinline, optimize("O2")))
test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Expected RTL pattern: (compare (plus i -1) 0)
     */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition.
     * Should also generate the same canonical pattern.
     */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with > 0 condition,
     * using a different variable name and bound.
     */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinguishing constant */
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    if (sum > 0) {
        /* This branch always taken, but compiler doesn't know that */
        printf("Loop sum: %d\n", sum);
    }
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 50;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    return 0;
}
