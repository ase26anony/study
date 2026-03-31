/* loop_pattern.c - Generates RTL pattern for do-loop conversion coverage */

/* Prevent constant propagation */
volatile int global_bound = 100;

/* Global to accumulate side effects */
int global_sum = 0;

/* Test function containing the target loop pattern */
void test_loop(int n) {
    int i;
    int local_sum = 0;
    
    /* First target loop: post-decrement in condition */
    /* Should generate: (set reg (plus reg -1)) followed by (set cc (compare (plus reg -1) 0)) */
    for (i = n; i > 0; i--) {
        /* Minimal side effect to prevent dead code elimination */
        local_sum += i;
        /* Alternative: volatile write or asm statement */
        /* asm volatile("" : : : "memory"); */
    }
    
    /* Second target loop with different variable */
    /* Provides another pattern-matching opportunity */
    int j = n;
    while (j-- > 0) {
        local_sum += j;
    }
    
    /* Third variation: separate decrement and compare */
    int k = n;
    while (1) {
        k--;
        if (k < 0) break;
        local_sum += k;
    }
    
    global_sum += local_sum;
}

/* Outer loop to increase optimization opportunities */
void outer_wrapper(int iterations) {
    for (int outer = 0; outer < iterations; outer++) {
        /* Pass volatile bound to prevent constant propagation */
        test_loop(global_bound + outer);
    }
}

int main() {
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int iterations = 3;
    
    /* Call the wrapper multiple times */
    outer_wrapper(iterations);
    
    /* Use the result to prevent complete optimization */
    return global_sum > 0 ? 0 : 1;
}
