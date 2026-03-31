/* test_loops.c
 * This program is designed to trigger the uncovered block in GCC's
 * loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrementing pattern: (compare (plus reg -1) 0).
 * The loops use volatile bounds to prevent constant propagation,
 * and are placed in a noinline function with O2 optimization.
 */

#include <stdio.h>

/* Static function with noinline attribute to isolate loops from main */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3) {
    volatile int sum = 0;  /* Prevent dead code elimination */
    
    /* Loop 1: Canonical decrementing loop with > 0 condition */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from Loop 1 */
    }
    
    /* Loop 3: Another decrementing loop with > 0 condition */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Different constant */
    }
    
    /* Use sum in a trivial way to prevent elimination */
    asm volatile ("" : : "r"(sum) : "memory");
}

int main(void) {
    /* Volatile bounds to prevent constant propagation */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to prevent dead code elimination of main */
    printf("Loop test completed.\n");
    
    return 0;
}
