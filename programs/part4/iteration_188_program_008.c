/* caller-save-test.c
 * Designed to trigger instruction movement into caller-save/restore sequences
 * and exercise the uncovered code in GCC's caller-save.cc lines 905-913
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
    long v5 = v4 + 1000;
    float v6 = (float)seed1 / 3.0f;
    float v7 = (float)seed2 * 1.5f;
    double v8 = (double)seed3 * 2.71828;
    double v9 = v8 / 1.41421;
    int v10 = v1 + v2;
    long v11 = v4 + v5;
    float v12 = v6 + v7;
    double v13 = v8 + v9;
    
    /* Additional variables to increase pressure further */
    int v14 = v3 * v10;
    double v15 = v13 * 2.0;
    
    /* Memory barrier to limit instruction reordering */
    asm volatile("" : : : "memory");
    
    /* This computation result is used AFTER the call */
    /* The compiler may need to move this instruction into save/restore sequence */
    int critical_value = v1 * v2 + v3 - v10;
    
    /* Call helper function - registers must be saved/restored */
    helper_func(v1, v8, v6, v4);
    
    /* Use the critical_value and other live values AFTER the call */
    /* This ensures they must survive across the call */
    int result = critical_value + v14 + (int)v12;
    
    /* More computations using variables that were live across the call */
    result += (int)(v15 / 3.0);
    result += v11 & 0xFFFF;
    
    /* Additional instructions after the call to ensure BB_END update */
    /* These create a basic block where the call is not the last instruction */
    volatile int final_check = result;
    if (final_check > 1000) {
        result = result % 1000;
    }
    
    return result;
}

/* Main function with loop to repeatedly trigger caller-save sequences */
int main(int argc, char *argv[]) {
    int i, total = 0;
    volatile int accumulator = 0;
    
    /* Get some seed values from command line or stdin */
    int seed1, seed2, seed3;
    
    if (argc >= 4) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
        seed3 = atoi(argv[3]);
    } else {
        /* Use stdin if no arguments */
        printf("Enter three integer seeds: ");
        scanf("%d %d %d", &seed1, &seed2, &seed3);
    }
    
    /* Loop to create repeated caller-save opportunities */
    for (i = 0; i < 100; i++) {
        /* Modify seeds slightly each iteration */
        int iter_seed1 = seed1 + i;
        int iter_seed2 = seed2 - i;
        int iter_seed3 = seed3 * (i % 5 + 1);
        
        /* Call worker function - this should trigger caller-save sequences */
        int result = worker_function(iter_seed1, iter_seed2, iter_seed3);
        
        /* Accumulate result in volatile to prevent optimization */
        accumulator += result;
        
        /* Memory clobber to prevent loop optimizations */
        asm volatile("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d (global_counter: %d)\n", accumulator, global_counter);
    
    return accumulator != 0 ? 0 : 1;
}
