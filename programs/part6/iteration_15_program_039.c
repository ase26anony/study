/* test_hwloop.c
 * This program is designed to trigger the hardware loop pattern recognition
 * logic in GCC's loop-doloop.cc, specifically the validation block that checks
 * for the canonical decrementing loop pattern (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from
 * early optimizations while ensuring loop passes run on it.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    
    /* Loop 1: Canonical decrement-to-zero with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }
    
    /* Loop 2: Decrement-to-zero with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinguishing constant */
    }
    
    /* Use sum to prevent elimination */
    if (sum > 1000) {
        printf("unlikely\n");  /* Never happens, but prevents optimization */
    }
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    return 0;
}
