/* test_hwloop.c
 * This program is designed to trigger the hardware loop pattern recognition
 * logic in GCC's loop-doloop.cc, specifically the uncovered validation block
 * that checks for the canonical decrement-and-branch pattern:
 * (compare (plus reg -1) 0)
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early
 * optimizations and ensure they reach the hardware loop detection phase.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent loop removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from first loop */
    }
    
    /* Loop 3: Another decrementing loop with > 0 condition, different counter */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Yet another constant */
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function isn't optimized away */
    printf("Test completed\n");
    
    return 0;
}
