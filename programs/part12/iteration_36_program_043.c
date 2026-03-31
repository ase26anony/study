/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;

/* External function to force runtime evaluation */
extern int get_arg_value(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional wrapper to force SIMT transformation */
    if (flag > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * (i % 7);
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * (i % 5);
        }
    }
    return sum;
}

/* Noinline function with goto to interact with artificial labels */
__attribute__((noinline)) 
double noinline_simd_loop(double *arr, int size, int skip_first) {
    double result = 0.0;
    int i = 0;
    
    /* Goto to force complex control flow */
    if (skip_first) {
        goto skip_simd;
    }
    
    /* This SIMD loop should trigger the uncovered code */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (i = 0; i < size; i++) {
        result += arr[i] * (i % 11);
    }
    
skip_simd:
    /* Another SIMD loop after label */
    #pragma omp simd simdlen(8) reduction(+:result)
    for (; i < size; i++) {
        result += arr[i] * (i % 13);
    }
    
    return result;
}

/* Main function with multiple SIMD constructs */
int main(int argc, char **argv) {
    int i, n = 1000;
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Parse command line for runtime control */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 100;
    use_simt_path = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Allocate and initialize arrays */
    int *int_arr = (int*)malloc(n * sizeof(int));
    float *float_arr = (float*)malloc(n * sizeof(float));
    double *double_arr = (double*)malloc(n * sizeof(double));
    
    for (i = 0; i < n; i++) {
        int_arr[i] = (i % 19) + 1;
        float_arr[i] = (float)(i % 23) * 0.1f;
        double_arr[i] = (double)(i % 29) * 0.01;
    }
    
    /* SIMD loop 1: In main() with conditional execution */
    if (simd_enabled && use_simt_path) {
        /* This should trigger the SIMT transformation */
        #pragma omp simd simdlen(16) reduction(+:int_result)
        for (i = 0; i < loop_count; i++) {
            int_result += int_arr[i] * ((i % 3) + 1);
        }
    } else {
        /* Alternative path */
        #pragma omp simd simdlen(8) reduction(+:int_result)
        for (i = 0; i < loop_count; i++) {
            int_result += int_arr[i] * ((i % 5) + 1);
        }
    }
    
    /* SIMD loop 2: In static function with runtime condition */
    int flag = get_arg_value() % 2;
    float_result = static_simd_reduction(float_arr, loop_count, flag);
    
    /* SIMD loop 3: In noinline function with goto */
    int skip = (get_arg_value() % 3 == 0) ? 1 : 0;
    double_result = noinline_simd_loop(double_arr, loop_count, skip);
    
    /* Additional OpenMP constructs to activate lowering infrastructure */
    #pragma omp parallel for simd
    for (i = 0; i < n/2; i++) {
        int_arr[i] *= 2;
    }
    
    /* Print results to prevent elimination */
    printf("Results: int=%d, float=%.2f, double=%.2f\n", 
           int_result, float_result, double_result);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}

/* External function implementation */
int get_arg_value(void) {
    static int counter = 0;
    return counter++ % 7;
}

/* Additional function with mixed OpenMP pragmas */
void mixed_omp_constructs(int *data, int n) {
    /* Parallel region */
    #pragma omp parallel
    {
        /* SIMD loop inside parallel region */
        #pragma omp for simd
        for (int i = 0; i < n; i++) {
            data[i] += omp_get_thread_num();
        }
    }
    
    /* Conditional SIMD with ternary-like structure */
    int chunk = (n > 1000) ? 128 : 64;
    #pragma omp simd simdlen(chunk)
    for (int i = 0; i < n; i++) {
        data[i] *= (i % 17);
    }
}
