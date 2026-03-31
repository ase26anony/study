/* test_hwloop.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating the canonical RTL pattern
 * (compare (plus reg -1) 0) for decrementing loops.
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early
 * optimizations while ensuring loop optimization passes run on it.
 */
static void __attribute__((noinline, optimize("O2"))) 
test_loops(volatile int bound1, volatile int bound2, volatile int bound3) 
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }
    
    /* Loop 3: Another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile ("" : : "r"(sum));
}

int main(void) 
{
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 300;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function isn't optimized away */
    printf("Test completed (loops executed)\n");
    
    return 0;
}
