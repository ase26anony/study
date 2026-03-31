/* test_loops.c
 * This program is designed to trigger the uncovered validation block in
 * GCC's loop-doloop.cc (lines 136-150) by generating loops that match the
 * canonical decrementing pattern: (compare (plus reg -1) 0).
 *
 * Compile with: gcc -O2 -fdump-rtl-loop2 -fno-unroll-loops -c test_loops.c
 * Or for ARM hardware loop testing: gcc -O2 -march=armv7-a -mthumb -fdump-rtl-doloop -c test_loops.c
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early
 * optimizations and ensure they reach the hardware loop detection phase.
 */
static void __attribute__((noinline, optimize("O2"))) 
test_loops(volatile int bound1, volatile int bound2, volatile int bound3) 
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Expected RTL pattern: (compare (plus i -1) 0)
     */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent loop removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition.
     * Also generates the canonical pattern.
     */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }
    
    /* Loop 3: Another decrementing loop with different bound.
     * Increases chances of pattern matching.
     */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile ("" : : "r"(sum) : "memory");
}

int main(void) 
{
    /* Volatile bounds prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function call isn't optimized away */
    printf("Loop test completed (check RTL dumps for pattern matching)\n");
    
    return 0;
}
