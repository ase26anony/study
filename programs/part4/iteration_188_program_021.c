/* caller-save-test.c
 * Designed to trigger instruction movement into caller-save/restore sequences
 * and test the basic block boundary update logic in GCC's caller-save.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent optimization */
volatile int global_counter = 0;

/* Non-inline helper function that will force a real call */
__attribute__((noinline)) 
void helper_func(int a, double b, float c, long d) {
    /* Simple side effect to prevent optimization */
    global_counter += a + (int)b + (int)c + (int)(d & 0xFF);
    /* Memory clobber to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Worker function with high register pressure around a call */
__attribute__((noinline))
int worker_function(int seed1, int seed2, int seed3) {
    /* Declare many variables of different types to create register pressure */
    int v1 = seed1 + 1;
    int v2 = seed2 * 2;
    int v3 = seed3 - 5;
    long v4 = (long)seed1 * seed2;
    long v5 = (long)seed2 * seed3;
    long v6 = (long)seed3 * seed1;
    float v7 = (float)seed1 / 3.0f;
    float v8 = (float)seed2 / 4.0f;
    float v9 = (float)seed3 / 5.0f;
    double v10 = (double)seed1 / 1.5;
    double v11 = (double)seed2 / 2.5;
    double v12 = (double)seed3 / 3.5;
    
    /* Additional variables to increase pressure further */
    int v13 = v1 + v2;
    int v14 = v2 + v3;
    float v15 = v7 + v8;
    double v16 = v10 + v11;
    
    /* Perform computations that create live ranges across the call */
    v1 = v1 * 2 + v13;
    v2 = v2 / 2 + v14;
    v4 = v4 + v5 - v6;
    v7 = v7 * 1.1f + v15;
    v10 = v10 * 1.01 + v16;
    
    /* This computation's result is used after the call - 
       its instruction may be moved into save/restore sequence */
    int critical_value = v1 * 3 + v2 * 2 + (int)v7;
    
    /* Memory barrier to limit reordering */
    asm volatile("" : : : "memory");
    
    /* Function call with many arguments - will clobber caller-saved registers */
    helper_func(v1, v10, v7, v4);
    
    /* Use the critical_value after the call - ensures it must survive */
    int result = critical_value + v3 + (int)v8 + (int)v11;
    
    /* More computations using variables that were live across the call */
    v12 = v12 + v10;
    v9 = v9 + v7;
    v6 = v6 + v4;
    
    /* Additional use to ensure variables stay live */
    result += (int)v12 + (int)v9 + (int)v6;
    
    /* Another call to increase pressure in loops */
    helper_func(result, v12, v9, v6);
    
    return result;
}

/* Main function that creates repeated pressure */
int main(int argc, char *argv[]) {
    int seed1, seed2, seed3;
    
    /* Get some initial values - prevent compile-time computation */
    if (argc >= 4) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
        seed3 = atoi(argv[3]);
    } else {
        /* Default seeds if no arguments provided */
        seed1 = 123;
        seed2 = 456;
        seed3 = 789;
    }
    
    volatile int accumulator = 0;
    
    /* Loop to create repeated caller-save/restore sequences */
    for (int i = 0; i < 100; i++) {
        /* Vary the seeds slightly each iteration */
        int iter_seed1 = seed1 + i;
        int iter_seed2 = seed2 + i * 2;
        int iter_seed3 = seed3 + i * 3;
        
        /* Call worker function - creates register pressure */
        int result = worker_function(iter_seed1, iter_seed2, iter_seed3);
        
        /* Use result to prevent optimization */
        accumulator += result;
        
        /* Additional computation that might force spill/restore decisions */
        if (i % 10 == 0) {
            /* Extra call with different arguments */
            helper_func(accumulator, (double)result, (float)i, i * 100L);
        }
    }
    
    /* Print result to create observable side effect */
    printf("Result: %d (global_counter: %d)\n", accumulator, global_counter);
    
    return 0;
}
