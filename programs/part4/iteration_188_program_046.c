/* caller-save-test.c
 * Test program to trigger caller-save register spilling and instruction
 * movement into save/restore sequences, specifically targeting the
 * uncovered lines in caller-save.cc that update instruction chains.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent optimization and create side effects */
volatile int global_counter = 0;

/* Non-inline helper function to force actual call instruction */
__attribute__((noinline)) 
void helper_function(int a, double b, float c, long d) {
    /* Simple side effect to prevent elimination */
    global_counter += a + (int)b + (int)c + (int)(d & 0xFF);
    
    /* Memory clobber to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Worker function with high register pressure around a call */
__attribute__((noinline))
int worker_function(int seed1, int seed2, int seed3, int iteration) {
    /* Declare many local variables of different types to create
     * register pressure across integer and floating-point registers */
    int v1 = seed1 + iteration;
    int v2 = seed2 * iteration;
    int v3 = seed3 - iteration;
    int v4 = v1 * v2 + v3;
    int v5 = v2 / (v1 + 1);
    
    long l1 = (long)seed1 * 1000 + iteration;
    long l2 = (long)seed2 * 2000 - iteration;
    long l3 = l1 + l2;
    long l4 = l1 - l2;
    
    float f1 = (float)seed1 * 1.5f + iteration;
    float f2 = (float)seed2 * 2.5f - iteration;
    float f3 = f1 + f2;
    float f4 = f1 * f2;
    float f5 = f3 / (f4 + 1.0f);
    
    double d1 = (double)seed1 * 1.7 + iteration;
    double d2 = (double)seed2 * 2.9 - iteration;
    double d3 = d1 + d2;
    double d4 = d1 * d2;
    double d5 = d3 / (d4 + 1.0);
    
    /* Additional variables to increase pressure */
    int v6 = v4 + v5;
    int v7 = v3 * v6;
    long l5 = l3 + l4;
    float f6 = f4 + f5;
    double d6 = d4 + d5;
    
    /* Memory barrier to limit reordering */
    asm volatile("" : : : "memory");
    
    /* Critical computation whose result is needed after the call.
     * This instruction may be moved into the save/restore sequence. */
    int critical_value = v7 + (int)f6 + (int)d6 + (int)(l5 & 0xFF);
    
    /* Call the helper function with some arguments.
     * Many registers are live across this call. */
    helper_function(v1, d1, f1, l1);
    
    /* Use the critical value and other live values after the call.
     * This ensures they must survive across the call. */
    int result = critical_value + v2 + v3 + (int)f2 + (int)d2;
    
    /* Additional computations to ensure the call is not at the
     * end of the basic block, allowing BB_END update. */
    result += v4 * 2;
    result -= v5 / 2;
    result += (int)(f3 * 10.0f);
    result += (int)(d3 * 5.0);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Main function that drives the test */
int main(int argc, char *argv[]) {
    /* Use command line arguments or stdin for initial values
     * to prevent compile-time evaluation */
    int seed1, seed2, seed3;
    
    if (argc >= 4) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
        seed3 = atoi(argv[3]);
    } else {
        /* Default seeds if no arguments provided */
        seed1 = 1234;
        seed2 = 5678;
        seed3 = 9012;
    }
    
    /* Volatile accumulator to prevent optimization */
    volatile int total_result = 0;
    
    /* Loop to create repeated caller-save scenarios */
    for (int i = 0; i < 100; i++) {
        /* Call worker function multiple times to increase
         * chances of triggering the uncovered code path */
        int result = worker_function(seed1 + i, seed2 - i, seed3 * (i % 10), i);
        
        /* Use volatile store to prevent dead code elimination */
        total_result += result;
        
        /* Occasionally call helper directly to vary the pattern */
        if (i % 7 == 0) {
            helper_function(result, (double)result, (float)result, (long)result);
        }
    }
    
    /* Print result to create observable side effect */
    printf("Final result: %d (global_counter: %d)\n", total_result, global_counter);
    
    return total_result != 0 ? 0 : 1;
}
