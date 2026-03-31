/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 100;

/* External function to force control flow */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper - forces SIMT transformation */
    if (enable) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += sqrtf(data[i]);
        }
    }
    
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_compute(double *arr, int size, int skip) {
    double result = 0.0;
    int i;
    
    /* Goto to interact with artificial labels */
    if (skip) goto skip_simd;
    
    /* This SIMD loop should trigger the uncovered code */
    #pragma omp simd simdlen(16) reduction(+:result) aligned(arr:64)
    for (i = 0; i < size; i++) {
        result += arr[i] * exp(arr[i] * 0.001);
    }
    
    goto end_compute;
    
skip_simd:
    /* Alternative path without SIMD */
    for (i = 0; i < size; i++) {
        result += arr[i];
    }
    
end_compute:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] = i % 100;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Threads: %d\n", omp_get_num_threads());
        }
    }
    
    free(data);
}

/* Main test driver */
int main(int argc, char **argv) {
    int N = 1000;
    int use_simt = 0;
    
    /* Parse command line for control variables */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (argc > 2) {
            use_simt = atoi(argv[2]);
        }
    }
    
    /* Force runtime condition */
    loop_control = N;
    use_simt_path = use_simt;
    
    /* Array initializations */
    int *int_data = (int*)malloc(N * sizeof(int));
    float *float_data = (float*)malloc(N * sizeof(float));
    double *double_data = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        int_data[i] = (i * 3) % 97;
        float_data[i] = (float)(i * 0.01);
        double_data[i] = (double)(i * 0.005);
    }
    
    /* Test 1: SIMD in main with conditional execution */
    int int_sum = 0;
    
    /* Complex condition to prevent static optimization */
    int condition = (argc > 0) && (N > 10) && (simd_enabled || use_simt_path);
    
    if (condition) {
        /* This should trigger the SIMT transformation */
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i] * 2;
        }
    } else {
        /* Alternative path */
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i];
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Static function with SIMD */
    float float_result = static_simd_reduction(float_data, N, use_simt);
    printf("Float reduction: %f\n", float_result);
    
    /* Test 3: Noinline function with goto */
    double double_result = noinline_simd_compute(double_data, N, !use_simt);
    printf("Double computation: %lf\n", double_result);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(N / 2);
    
    /* Additional test with ternary operator */
    int alt_sum = 0;
    (use_simt) ? 
    (
        #pragma omp simd simdlen(16) reduction(+:alt_sum)
        for (int i = 0; i < N; i += 2) {
            alt_sum += int_data[i];
        }
    ) : 
    (
        #pragma omp simd simdlen(8) reduction(+:alt_sum)
        for (int i = 1; i < N; i += 2) {
            alt_sum += int_data[i];
        }
    );
    
    printf("Alternate sum: %d\n", alt_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
