/* caller-save-test.c
 * Designed to trigger instruction movement into caller-save/restore sequences
 * and exercise the uncovered code in caller-save.cc lines 905-913
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent optimization */
volatile int global_counter = 0;

/* Non-inline helper function to force actual call generation */
__attribute__((noinline)) 
void helper_function(int a, double b, float c, long d) {
    /* Simple side effect to prevent elimination */
    global_counter += a + (int)b + (int)c + (int)(d & 0xFF);
    
    /* Memory clobber to prevent reordering */
    asm volatile("" : : : "memory");
}

/* Worker function with high register pressure */
__attribute__((noinline))
int worker_function(int seed1, int seed2, int seed3, int iteration) {
    /* Declare many variables of mixed types to create register pressure */
    int v1 = seed1 + iteration;
    int v2 = seed2 * 2;
    int v3 = seed3 - iteration;
    long v4 = (long)seed1 * seed2;
    long v5 = v4 + 12345;
    float v6 = (float)seed1 * 0.5f;
    float v7 = (float)seed2 * 1.5f;
    float v8 = v6 + v7;
    double v9 = (double)seed3 * 2.5;
    double v10 = v9 / 3.14159;
    int v11 = v1 + v2;
    long v12 = v4 * v5;
    float v13 = v6 * v7;
    double v14 = v9 + v10;
    
    /* Additional variables to increase pressure further */
    int v15 = v11 ^ v3;
    float v16 = v8 + v13;
    double v17 = v14 * 2.0;
    
    /* Perform computations that create dependencies */
    v1 = v1 * v2 + v3;
    v4 = v4 - v5 * 2;
    v6 = v6 / (v7 + 1.0f);
    v9 = v9 - v10;
    
    /* Memory barrier to limit reordering */
    asm volatile("" : : : "memory");
    
    /* Call helper function - registers must be saved/restored */
    helper_function(v1, v9, v6, v4);
    
    /* Use values computed before the call after the call */
    /* This creates liveness across the call */
    int result = v1 + v2;
    result += (int)v6;
    result += (int)(v4 & 0xFFFF);
    result += (int)v9;
    
    /* More computations using pre-call values */
    v15 = v15 + result;
    v16 = v16 * 2.0f;
    v17 = v17 / 1.5;
    
    /* Additional instructions after the call */
    /* Ensures the call is not at the end of the basic block */
    result += v15;
    result += (int)v16;
    result += (int)v17;
    
    /* Volatile store to prevent dead code elimination */
    volatile int final_result = result;
    
    return final_result;
}

/* Main function with loop to create repeated save/restore decisions */
int main(int argc, char *argv[]) {
    int seed1, seed2, seed3;
    volatile int total = 0;
    
    /* Get initial values from command line or stdin */
    if (argc >= 4) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
        seed3 = atoi(argv[3]);
    } else {
        printf("Enter three integer seeds: ");
        if (scanf("%d %d %d", &seed1, &seed2, &seed3) != 3) {
            seed1 = 123;
            seed2 = 456;
            seed3 = 789;
        }
    }
    
    /* Loop to create repeated caller-save decisions */
    for (int i = 0; i < 100; i++) {
        /* Call worker function multiple times */
        int result = worker_function(seed1 + i, seed2 - i, seed3 * i, i);
        
        /* Use result to prevent elimination */
        total += result;
        
        /* Occasionally modify seeds to prevent loop unrolling */
        if (i % 7 == 0) {
            seed1 ^= result & 0xFF;
        }
    }
    
    /* Print result to create observable side effect */
    printf("Result: %d (global_counter: %d)\n", total, global_counter);
    
    return total != 0 ? 0 : 1;
}
