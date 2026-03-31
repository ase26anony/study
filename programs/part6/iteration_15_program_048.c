/* test_loops.c
 * This program is designed to trigger the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) by creating canonical decrementing
 * loop patterns that match the RTL pattern (compare (plus reg -1) 0).
 * The loops are placed in a noinline function with O2 optimization
 * to ensure they reach the hardware loop detection pass.
 */

#include <stdio.h>

/* Static function with noinline to prevent inlining and ensure
 * optimization passes run on the function body. The O2 optimization
 * attribute ensures loop optimizations are applied. */
static void __attribute__((noinline, optimize("O2"))) 
test_loops(volatile int bound1, volatile int bound2, volatile int bound3) 
{
    volatile int sum = 0;
    
    /* First loop: canonical decrement to zero with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Second loop: decrement to zero with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish loops */
    }
    
    /* Third loop: another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    asm volatile ("" : : "r"(sum) : "memory");
}

int main(void) 
{
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function isn't optimized away */
    printf("Loop test completed\n");
    
    return 0;
}
