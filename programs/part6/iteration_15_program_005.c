/* test_hwloop.c
 * This program is designed to trigger the hardware loop pattern recognition
 * logic in GCC's loop-doloop.cc, specifically the uncovered validation block
 * that checks for the canonical decrement-and-branch pattern:
 * (compare (plus reg -1) 0)
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early
 * optimizations while ensuring they are processed by the loop optimization passes.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;  /* Prevent elimination of loop bodies */
    
    /* Loop 1: Canonical decrement to zero with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;
    }
    
    /* Loop 2: Decrement to zero with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }
    
    /* Loop 3: Another decrement loop with > comparison, different variable */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    asm volatile("" : : "r"(sum) : "memory");
}

int main(void) {
    /* Volatile bounds prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 300;
    
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function call isn't optimized away */
    printf("Test completed (bounds: %d, %d, %d)\n", bound1, bound2, bound3);
    
    return 0;
}
