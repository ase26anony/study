/* test_hardware_loops.c
 * Targets GCC's loop-doloop.cc validation logic for hardware loop pattern recognition.
 * The goal is to trigger the uncovered block checking for (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early optimizations */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, 
                                                                 volatile int bound2, 
                                                                 volatile int bound3)
{
    volatile int sum = 0;  /* Prevent dead code elimination */
    
    /* Loop 1: Canonical decrementing for loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinguishing constant */
    }
    
    /* Use sum to prevent elimination - volatile ensures side effect */
    (void)sum;
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 300;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to prevent dead code elimination of main */
    printf("Hardware loop pattern test completed\n");
    
    return 0;
}
