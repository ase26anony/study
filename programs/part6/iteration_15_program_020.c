/* test_hardware_loops.c
 * This program is designed to trigger the hardware loop pattern recognition
 * logic in GCC's loop-doloop.cc, specifically the validation block that checks
 * for the canonical decrementing loop pattern (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early
 * optimizations while ensuring they reach the hardware loop detection phase.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    
    /* Loop 1: Canonical decrement-to-zero with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }
    
    /* Loop 2: Decrement-to-zero with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from Loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum to prevent elimination */
    if (sum > 1000) {
        printf("unlikely\n");  /* Never executes, but prevents optimization */
    }
}

int main(void) {
    /* Volatile bounds prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function isn't eliminated */
    printf("Test completed\n");
    
    return 0;
}
