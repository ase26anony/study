/* Target: GCC loop inversion optimization pattern matching
 * Specifically targets lines 136-150 in loop-doloop.cc
 * Pattern: (set cc (compare (plus reg -1) 0))
 */

#include <stdio.h>

/* Global to accumulate side effects */
volatile int global_sum = 0;

/* Test function containing the target loop pattern */
void test_loop_pattern(int n) {
    int i = n;
    int local_sum = 0;
    
    /* First target loop: post-decrement in condition */
    /* Should generate: (set reg_i (plus reg_i -1)) 
     * followed by: (set cc (compare (plus reg_i -1) 0)) */
    while (i-- > 0) {
        /* Minimal side effect to prevent dead code elimination */
        local_sum += 1;
        asm volatile("" : : : "memory");  /* Memory barrier */
    }
    
    /* Second target loop with different counter */
    /* Provides another pattern matching opportunity */
    int j = n / 2;
    while (j-- > 0) {
        local_sum += 2;
        asm volatile("" : : : "memory");
    }
    
    /* Third loop: for loop with post-decrement */
    int k = n / 3;
    for (; k > 0; k--) {
        local_sum += 3;
        asm volatile("" : : : "memory");
    }
    
    /* Store result to volatile global to ensure side effect */
    global_sum += local_sum;
}

/* Another test function with similar pattern */
void another_test(int m) {
    int x = m;
    int temp = 0;
    
    /* Loop with separate decrement and compare */
    while (x) {
        temp += x;
        x--;
        asm volatile("" : : : "memory");
    }
    
    global_sum += temp;
}

int main() {
    /* Volatile initial bounds to prevent constant propagation */
    volatile int N = 1000;
    volatile int M = 500;
    
    /* Call test functions multiple times to increase optimization opportunities */
    for (int outer = 0; outer < 10; outer++) {
        test_loop_pattern(N);
        another_test(M);
        
        /* Additional loop patterns with different bounds */
        int p = N + outer;
        while (p-- > 0) {
            global_sum += outer;
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use the result to prevent complete optimization */
    printf("Result: %d\n", global_sum);
    
    return 0;
}
