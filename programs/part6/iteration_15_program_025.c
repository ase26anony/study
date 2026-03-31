/* test_hardware_loops.c
 * This program is designed to trigger the hardware loop pattern recognition
 * in GCC's loop optimization passes, specifically targeting the uncovered
 * validation block in loop-doloop.cc that checks for the canonical
 * decrement-and-branch pattern (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate the loops
 * from early optimizations and ensure they reach the hardware loop
 * detection phase.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;
    
    /* First loop: canonical decrement to zero with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Second loop: decrement to zero with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from first loop */
    }
    
    /* Third loop: another decrementing loop with > comparison */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinct constant */
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    asm volatile("" : : "r"(sum) : "memory");
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function isn't optimized away */
    printf("Test completed (loops executed)\n");
    
    return 0;
}
