#include <stdio.h>
#include <stdlib.h>

/* Global variable to prevent optimization */
volatile int global_counter = 0;

/* Non-inline helper function to force actual call */
__attribute__((noinline)) void helper_func(int a, double b, float c, long d) {
    /* Simple side effect to prevent removal */
    global_counter += a + (int)b + (int)c + (int)(d & 0xFF);
    /* Memory barrier to limit reordering */
    asm volatile("" ::: "memory");
}

/* Worker function with register pressure */
__attribute__((noinline)) void worker_function(int seed1, int seed2, int iterations) {
    /* Declare many variables of different types to create register pressure */
    int v1 = seed1;
    int v2 = seed2;
    int v3 = v1 * v2;
    int v4 = v1 + v2;
    long v5 = (long)v1 * v2 * 3;
    long v6 = v5 + v1 - v2;
    float v7 = (float)v1 * 0.5f;
    float v8 = (float)v2 * 1.5f;
    float v9 = v7 + v8;
    double v10 = (double)v1 * 2.5;
    double v11 = (double)v2 * 3.5;
    double v12 = v10 + v11;
    int v13 = v3 + v4;
    long v14 = v5 + v6;
    float v15 = v7 * v8;
    
    /* Use all variables in computations before call */
    int pre_sum = v1 + v2 + v3 + v4 + v13;
    long pre_long = v5 + v6 + v14;
    float pre_float = v7 + v8 + v9 + v15;
    double pre_double = v10 + v11 + v12;
    
    /* This instruction's result is used after the call */
    /* It may be moved into the save/restore sequence */
    int critical_value = pre_sum * 2 + (int)pre_float;
    
    /* Call helper - many registers must be saved */
    helper_func(pre_sum, pre_double, pre_float, pre_long);
    
    /* Use values after call - they must survive */
    int post_sum = critical_value + v1;  // Uses critical_value computed before call
    long post_long = pre_long + v5;
    float post_float = pre_float + v7;
    double post_double = pre_double + v10;
    
    /* More computations to ensure BB doesn't end at call */
    v1 = post_sum % 100;
    v2 = (int)post_double % 100;
    
    /* Use volatile to prevent optimization */
    volatile int result = v1 + v2 + (int)post_float + (int)(post_long % 100);
    
    /* Print to create side effect */
    if (iterations % 1000 == 0) {
        printf("Intermediate: %d\n", result);
    }
}

/* Main function with loop to increase pressure */
int main(int argc, char *argv[]) {
    int seed1, seed2;
    
    /* Get seeds from command line or stdin */
    if (argc >= 3) {
        seed1 = atoi(argv[1]);
        seed2 = atoi(argv[2]);
    } else {
        printf("Enter two integer seeds: ");
        scanf("%d %d", &seed1, &seed2);
    }
    
    int iterations = 10000;
    
    /* Loop to create repeated caller-save scenarios */
    for (int i = 0; i < iterations; i++) {
        /* Modify seeds slightly each iteration */
        int mod1 = seed1 + i;
        int mod2 = seed2 - i;
        
        /* Call worker function - will generate register pressure */
        worker_function(mod1, mod2, i);
        
        /* Additional computation to prevent tail call optimization */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Final counter: %d\n", global_counter);
    return global_counter & 0xFF;
}
