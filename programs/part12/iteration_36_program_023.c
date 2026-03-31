/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc, lines 2941-2975. This creates conditional wrappers with
   gbind, artificial labels, and _SIMT_ clause attachment for OpenMP SIMD loops. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 100;

/* External function to force control flow complexity */
extern int get_random(void);

/* Static helper with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper - forces gbind creation */
    if (enable) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * 0.5f;
        }
    }
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int flag) {
    double result = 0.0;
    
    /* Complex control flow with goto - interacts with artificial labels */
    if (flag > 0) {
        goto compute;
    } else {
        goto skip;
    }
    
compute:
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(4) reduction(+:result) aligned(data:32)
    for (int i = 0; i < n; i += 2) {  /* Non-unit stride */
        result += data[i];
    }
    goto finish;
    
skip:
    /* Alternative SIMD loop */
    #pragma omp simd simdlen(8) reduction(+:result)
    for (int i = 1; i < n; i += 2) {
        result += data[i] * 2.0;
    }
    
finish:
    return result;
}

/* Main function with multiple SIMD constructs */
int main(int argc, char **argv) {
    /* Parse command line for runtime control */
    int loop_size = (argc > 1) ? atoi(argv[1]) : 1000;
    int use_simt = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Allocate and initialize arrays with different types */
    int *int_data = (int*)malloc(loop_size * sizeof(int));
    float *float_data = (float*)malloc(loop_size * sizeof(float));
    double *double_data = (double*)malloc(loop_size * sizeof(double));
    
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = i % 100;
        float_data[i] = (i % 100) * 0.5f;
        double_data[i] = (i % 100) * 0.25;
    }
    
    /* 1. Integer SIMD reduction in main with volatile control */
    int int_sum = 0;
    
    /* Ternary operator for conditional execution */
    (use_simt_path || simd_enabled) ? 
    (
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_control && i < loop_size; i++) {
            int_sum += int_data[i];
        }
    ) : 
    (
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_size; i++) {
            int_sum += int_data[i] * 2;
        }
    );
    
    /* 2. Float SIMD in static function with nested condition */
    float float_sum = 0.0f;
    for (int iter = 0; iter < 3; iter++) {
        /* Nested condition forces complex control flow */
        if (iter == use_simt) {
            float_sum += static_simd_reduction(float_data, loop_size, iter % 2);
        }
    }
    
    /* 3. Double SIMD in noinline function with goto */
    double double_sum = noinline_simd_reduction(double_data, loop_size, use_simt);
    
    /* 4. Additional mixed OpenMP construct to activate lowering infrastructure */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < loop_size/2; i++) {
        int_data[i] += 1;
    }
    
    /* Print results to prevent elimination */
    printf("Results: int=%d float=%.2f double=%.2f\n", 
           int_sum, float_sum, double_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Additional function to force different DECL_CONTEXT */
void __attribute__((cold)) cold_function_simd(void) {
    volatile int cold_flag = 1;
    int arr[100];
    
    for (int i = 0; i < 100; i++) arr[i] = i;
    
    /* SIMD in cold function - different context */
    if (cold_flag) {
        int sum = 0;
        #pragma omp simd simdlen(2) reduction(+:sum)
        for (int i = 0; i < 100; i++) {
            sum += arr[i];
        }
        printf("Cold sum: %d\n", sum);
    }
}
