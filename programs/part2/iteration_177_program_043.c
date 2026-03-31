/* loop-doloop-test.c
 * Test program to trigger GCC's loop inversion optimization pattern
 * that matches decrement-and-branch sequence for do-while conversion.
 */

#include <stdio.h>

/* Global variable to prevent complete optimization */
volatile int global_sum = 0;

/* Test function containing the target loop pattern */
void test_loop(int n) {
    int i, j;
    int local_sum = 0;
    
    /* First target loop: post-decrement in condition */
    /* Should generate: (set reg (plus reg -1)) followed by compare against 0 */
    for (i = n; i > 0; i--) {
        /* Minimal side effect to prevent dead code elimination */
        local_sum += i;
        /* Use volatile asm to ensure loop body isn't optimized away */
        asm volatile("" : : : "memory");
    }
    
    /* Second target loop: different pattern, same concept */
    /* Using while loop with post-decrement */
    j = n * 2;  /* Different bound to create different pattern */
    while (j-- > 0) {
        local_sum += j;
        asm volatile("" : : : "memory");
    }
    
    /* Store result to prevent optimization */
    global_sum = local_sum;
}

/* Another test function with similar pattern */
void test_loop2(int m) {
    int k = m;
    int temp = 0;
    
    /* Third target loop */
    do {
        temp += k;
        asm volatile("" : : : "memory");
    } while (k-- > 0);
    
    global_sum += temp;
}

int main() {
    /* Volatile initial bound to prevent constant propagation */
    volatile int N = 100;
    volatile int M = 50;
    
    /* Call test functions multiple times to increase pattern matching opportunities */
    for (int outer = 0; outer < 3; outer++) {
        test_loop(N);
        test_loop2(M);
    }
    
    /* Use the result to prevent complete optimization */
    printf("Result: %d\n", global_sum);
    
    return 0;
}
