/* test_hwloop.c
 * This program is designed to trigger the hardware loop pattern recognition
 * logic in GCC's loop-doloop.cc, specifically the validation block that checks
 * for the canonical decrement-and-branch pattern (compare (plus reg -1) 0).
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from
 * early optimizations and ensure they reach the hardware loop detection pass.
 */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;
    
    /* Loop 1: Canonical decrementing loop with > 0 condition.
     * Should generate: (compare (plus i -1) 0)
     */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent removal */
    }
    
    /* Loop 2: Decrementing loop with != 0 condition.
     * Should also generate: (compare (plus j -1) 0)
     */
    for (int j = bound2; j != 0; j--) {
        sum += 2;
    }
    
    /* Loop 3: Another decrementing loop with > 0 condition.
     * Increases chances of pattern matching.
     */
    for (int k = bound3; k > 0; k--) {
        sum += 3;
    }
    
    /* Use sum to prevent dead code elimination */
    printf("Dummy output: %d\n", sum);
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 300;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    return 0;
}
