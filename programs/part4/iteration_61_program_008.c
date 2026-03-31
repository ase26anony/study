/* test_modulo_sched.c
 * 
 * This program creates loops with specific patterns to trigger
 * GCC's modulo scheduler debug output for dependency edges.
 * Compile with: gcc -O3 -fmodulo-sched -fdump-rtl-sched -c test_modulo_sched.c
 */

#include <stdio.h>
#include <math.h>

#define N 256
#define M 128

/* Prevent optimization of critical variables */
static volatile int sink;

/* Function with multiple loops exhibiting different dependency patterns */
double compute_loop_patterns(void) {
    /* Pattern 1: Integer carried dependency with arithmetic operations */
    int array1[N];
    int array2[N];
    int result1 = 1;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i % 37 + 1;
        array2[i] = (i * 7) % 41 + 1;
    }
    
    /* Loop 1: Simple carried dependency (distance-1) with integer multiply */
    /* result1_i depends on result1_{i-1} */
    for (int i = 1; i < N; i++) {
        /* Carried dependency: result1 from previous iteration used in current */
        result1 = result1 * 3 + array1[i] * array2[i-1];
        /* Additional operation to create more scheduling pressure */
        array1[i] = (array1[i-1] + result1) % 100;
    }
    
    /* Pattern 2: Floating-point carried dependency with high-latency operations */
    double fp_array[M];
    double fp_result = 1.0;
    double fp_accum = 0.0;
    
    /* Initialize floating-point array */
    for (int i = 0; i < M; i++) {
        fp_array[i] = sin(i * 0.1) + 2.0;
    }
    
    /* Loop 2: Multiple carried dependencies with floating-point division/multiplication */
    /* High-latency operations that compete for FP units */
    for (int i = 1; i < M; i++) {
        /* Carried dependency chain 1: fp_result depends on previous iteration */
        fp_result = fp_result / 3.14159 * fp_array[i];
        
        /* Carried dependency chain 2: fp_accum depends on previous iteration */
        fp_accum = fp_accum + fp_result * fp_array[i-1];
        
        /* Additional high-latency operation */
        fp_array[i] = fp_array[i] * exp(-fp_accum * 0.01);
    }
    
    /* Pattern 3: Nested dependencies with mixed operations */
    int mixed_array[N/2];
    double mixed_result = 0.0;
    
    for (int i = 0; i < N/2; i++) {
        mixed_array[i] = (i * 13) % 29;
    }
    
    /* Loop 3: Complex dependency web */
    for (int i = 2; i < N/2; i++) {
        /* Multiple inter-dependent carried dependencies */
        int temp = mixed_array[i-1] * mixed_array[i-2];
        mixed_result = mixed_result + temp * 0.5;
        mixed_array[i] = (mixed_array[i] + (int)mixed_result) % 1000;
        
        /* Additional arithmetic to create scheduling pressure */
        mixed_result = mixed_result * 1.01 - sin(mixed_array[i] * 0.001);
    }
    
    /* Combine results to prevent dead code elimination */
    double final_result = result1 + fp_result + fp_accum + mixed_result;
    
    /* Use volatile sink to ensure computation isn't optimized away */
    sink = (int)final_result;
    
    return final_result;
}

/* Pattern 4: Loop with if-converted dependencies */
int conditional_dependency_loop(void) {
    int cond_array[N];
    int cond_sum = 0;
    int cond_prod = 1;
    
    for (int i = 0; i < N; i++) {
        cond_array[i] = i * 3;
    }
    
    /* Loop with conditional carried dependencies */
    for (int i = 1; i < N; i++) {
        /* Carried dependency that's conditionally used */
        if (cond_array[i] > cond_array[i-1]) {
            cond_sum = cond_sum + cond_prod;
        }
        
        /* Unconditional carried dependency */
        cond_prod = cond_prod * 2 + cond_array[i-1] % 7;
        
        /* Another carried dependency through array */
        cond_array[i] = cond_array[i] + cond_sum % 100;
    }
    
    return cond_sum + cond_prod;
}

/* Main function to drive the computation */
int main(void) {
    double result1 = compute_loop_patterns();
    int result2 = conditional_dependency_loop();
    
    /* Print results to create observable side effects */
    printf("Result 1: %f\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
