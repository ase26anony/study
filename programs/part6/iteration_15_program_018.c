/* test_hardware_loops.c
 * Generates canonical decrementing loop patterns to trigger
 * hardware loop validation in GCC's loop-doloop.cc
 */

#include <stdio.h>

/* Static function with optimization attributes to preserve loop patterns */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinguishing constant */
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile ("" : : "r"(sum) : "memory");
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to ensure execution */
    printf("Loop test completed\n");
    
    return 0;
}
