/* test_hardware_loops.c
 * Designed to trigger GCC's hardware loop pattern recognition
 * Specifically targets the validation of (compare (plus reg -1) 0) pattern
 */

#include <stdio.h>

/* Static function with optimization attributes to preserve loop structure */
static void __attribute__((noinline, optimize("O2"))) test_loops(volatile int bound1, volatile int bound2, volatile int bound3)
{
    volatile int sum = 0;
    
    /* First loop: decrementing counter with > 0 comparison */
    for (volatile int i = bound1; i > 0; i--) {
        sum += 1;  /* Simple side effect to prevent optimization */
    }
    
    /* Second loop: decrementing counter with != 0 comparison */
    for (int j = bound2; j != 0; j--) {
        sum += 2;  /* Different constant to distinguish from first loop */
    }
    
    /* Third loop: another decrementing pattern */
    for (int k = bound3; k > 0; k--) {
        sum += 3;  /* Another distinct side effect */
    }
    
    /* Use sum to prevent dead code elimination */
    if (sum > 1000) {
        printf("Unexpected large sum\n");
    }
}

int main(void)
{
    /* Volatile bounds to prevent constant propagation */
    volatile int bound1 = 100;
    volatile int bound2 = 200;
    volatile int bound3 = 150;
    
    /* Call the function containing our target loops */
    test_loops(bound1, bound2, bound3);
    
    /* Print something to ensure execution */
    printf("Loop test completed\n");
    
    return 0;
}
