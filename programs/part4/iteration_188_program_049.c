/* Compile with: gcc -O2 -fcaller-saves -fno-optimize-sibling-calls -fprofile-arcs -ftest-coverage -o test_caller_save test_caller_save.c */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent optimization */
volatile int global_counter = 0;

/* Non-inline helper function that will force a real call */
__attribute__((noinline)) 
void helper_function(int a, double b, float c, long d) {
    /* Simple side effect to prevent optimization */
    global_counter += a + (int)b + (int)c + (int)(d & 0xFF);
    
    /* Memory clobber to prevent reordering */
    asm volatile("" ::: "memory");
}

/* Worker function with high register pressure */
__attribute__((noinline))
int worker_function(int seed1, int seed2, int seed3, int seed4) {
    /* Declare many variables of different types to create register pressure */
    int v1 = seed1 + 1;
    int v2 = seed2 * 2;
    int v3 = seed3 - seed4;
    int v4 = seed1 * seed2;
    int v5 = seed3 + seed4;
    
    long l1 = (long)seed1 * seed2 * 100;
    long l2 = (long)seed3 * seed4 * 200;
    long l3 = (long)v1 * v2 * 300;
    long l4 = (long)v3 * v4 * 400;
    
    float f1 = (float)seed1 * 1.1f;
    float f2 = (float)seed2 * 2.2f;
    float f3 = (float)seed3 * 3.3f;
    float f4 = (float)seed4 * 4.4f;
    float f5 = f1 + f2;
    float f6 = f3 * f4;
    
    double d1 = (double)seed1 * 1.234567;
    double d2 = (double)seed2 * 2.345678;
    double d3 = (double)seed3 * 3.456789;
    double d4 = (double)seed4 * 4.567890;
    double d5 = d1 + d2;
    double d6 = d3 * d4;
    
    /* Perform computations that use all variables before the call */
    int int_result = v1 + v2 + v3 + v4 + v5;
    long long_result = l1 - l2 + l3 - l4;
    float float_result = f1 * f2 + f3 / f4 + f5 - f6;
    double double_result = d1 / d2 + d3 * d4 - d5 + d6;
    
    /* Additional intermediate computations to create more live values */
    int intermediate1 = int_result * 2;
    long intermediate2 = long_result / 3;
    float intermediate3 = float_result * 1.5f;
    double intermediate4 = double_result / 2.0;
    
    /* This variable's computation might be moved into save/restore sequence */
    int critical_value = intermediate1 + (int)intermediate3;
    
    /* Memory barrier to limit reordering */
    asm volatile("" ::: "memory");
    
    /* Function call - many registers must be saved/restored */
    helper_function(int_result, double_result, float_result, long_result);
    
    /* Use values after the call - they must survive across call */
    volatile int after_call_use = critical_value;  /* This use might force instruction movement */
    
    /* More computations using pre-call values */
    int final_int = intermediate1 + after_call_use;
    long final_long = intermediate2 + (long)intermediate4;
    float final_float = intermediate3 + (float)intermediate4;
    
    /* Return a value using all computed results */
    return final_int + (int)final_long + (int)final_float + global_counter;
}

/* Main function with loop to create repeated caller-save scenarios */
int main(int argc, char *argv[]) {
    int i, result = 0;
    volatile int accumulator = 0;
    
    /* Get some seed values from command line or stdin */
    int seed1, seed2, seed3, seed4;
    
    if (argc >= 5) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
        seed3 = atoi(argv[3]);
        seed4 = atoi(argv[4]);
    } else {
        /* Use stdin if no arguments */
        printf("Enter 4 seed integers: ");
        if (scanf("%d %d %d %d", &seed1, &seed2, &seed3, &seed4) != 4) {
            seed1 = 1; seed2 = 2; seed3 = 3; seed4 = 4;
        }
    }
    
    /* Loop to create repeated caller-save opportunities */
    for (i = 0; i < 100; i++) {
        /* Modify seeds slightly each iteration */
        int iter_seed1 = seed1 + i;
        int iter_seed2 = seed2 - i;
        int iter_seed3 = seed3 * (i % 5 + 1);
        int iter_seed4 = seed4 / (i % 3 + 1);
        
        /* Call worker function - this should trigger caller-save sequences */
        result = worker_function(iter_seed1, iter_seed2, iter_seed3, iter_seed4);
        
        /* Accumulate results in volatile to prevent optimization */
        accumulator += result;
        
        /* Additional computation to ensure basic block continues after call */
        if (i % 10 == 0) {
            accumulator -= global_counter;
        }
    }
    
    printf("Final accumulator value: %d\n", accumulator);
    printf("Global counter: %d\n", global_counter);
    
    return accumulator != 0 ? 0 : 1;
}
