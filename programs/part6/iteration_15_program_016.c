/* test_hardware_loops.c
 * Designed to trigger the uncovered validation block in GCC's loop-doloop.cc
 * The block validates the RTL pattern (compare (plus reg -1) 0) for hardware loop conversion.
 */

#include <stdio.h>

/* Static function with optimization attributes to isolate loops from early optimizations */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;
    
    /* First loop: canonical decrement-to-zero pattern with > comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent elimination */
    }
    
    /* Second loop: decrement-to-zero pattern with != comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from first loop */
    }
    
    /* Third loop: another decrement-to-zero pattern */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinguishing constant */
    }
    
    /* Use sum to prevent dead code elimination */
    asm volatile ("" : : "r"(sum) : "memory");
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation and loop unrolling */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 300;
    
    /* Call the function containing the target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print a dummy value to ensure the function isn't optimized away */
    printf("Test completed (loops executed)\n");
    
    return 0;
}
