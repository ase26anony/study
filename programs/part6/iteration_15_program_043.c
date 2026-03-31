/* test_hardware_loops.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrement-and-branch pattern: (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with attributes to prevent inlining and ensure O2
 * optimization runs on the function body. This isolates the loops from
 * early optimizations that might remove them before hardware loop detection.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2) {
    volatile int sum = 0;  /* Side-effect accumulator to prevent loop removal */
    
    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Expected RTL pattern: (compare (plus i -1) 0)
     */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Trivial side effect */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition.
     * Also generates the same canonical pattern.
     */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from first loop */
    }
    
    /* Loop 3: Additional decrementing loop with different variable name */
    for (int k = bound1 + bound2; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum in a trivial way to prevent dead code elimination */
    asm volatile("" : : "r"(sum) : "memory");
}

int main(void) {
    /* Volatile bounds to prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2);
    
    /* Print a dummy value to ensure the function call isn't optimized away */
    printf("Test completed (loops executed)\n");
    
    return 0;
}
