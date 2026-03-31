#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to prevent inlining */
extern void external_control(int *flag) __attribute__((noinline));

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper around SIMD loop */
    if (enable > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i] * data[i];
        }
    } else {
        /* Alternative path without SIMD */
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    }
    return sum;
}

/* Noinline function with goto and SIMD */
__attribute__((noinline)) 
double noinline_simd_compute(double *arr, int size, int skip) {
    double result = 0.0;
    int i;
    
    /* Use goto to create complex control flow */
    if (skip) {
        goto skip_simd;
    }
    
    /* SIMD loop with conditional execution */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (i = 0; i < size; i++) {
        result += sin(arr[i]) * cos(arr[i]);
    }
    
    goto end_computation;
    
skip_simd:
    /* Non-SIMD fallback */
    for (i = 0; i < size; i++) {
        result += arr[i];
    }
    
end_computation:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_operations(int *data, int n) {
    int i;
    
    /* Regular parallel for */
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    /* Conditional SIMD with volatile control */
    volatile int simd_flag = simd_enabled;
    
    if (simd_flag || use_simt_path) {
        int sum = 0;
        
        /* This should trigger SIMT transformation */
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += data[i] % 17;
        }
        
        printf("SIMD reduction result: %d\n", sum);
    }
}

/* Main function with various SIMD constructs */
int main(int argc, char **argv) {
    int N = 10000;
    
    /* Parse command line arguments for runtime control */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (argc > 2) {
            simd_enabled = atoi(argv[2]);
        }
        if (argc > 3) {
            use_simt_path = atoi(argv[3]);
        }
    }
    
    /* Allocate and initialize arrays with different types */
    int *int_data = (int*)malloc(N * sizeof(int));
    float *float_data = (float*)malloc(N * sizeof(float));
    double *double_data = (double*)malloc(N * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        int_data[i] = (i * 3) % 97;
        float_data[i] = (float)i / 100.0f;
        double_data[i] = sin((double)i / 50.0);
    }
    
    /* 1. Integer SIMD reduction in main with conditional */
    int int_sum = 0;
    int use_simd = (simd_enabled > 0) ? 1 : 0;
    
    /* Complex condition to force conditional wrapper */
    if ((use_simd && N > 100) || (use_simt_path && N % 2 == 0)) {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < N; i += 2) {
            int_sum += int_data[i] * 2 - int_data[i+1];
        }
    } else {
        /* Fallback path */
        for (int i = 0; i < N; i++) {
            int_sum += int_data[i];
        }
    }
    
    printf("Integer reduction: %d\n", int_sum);
    
    /* 2. Float SIMD in static function with runtime bounds */
    int loop_bound = (N > loop_control) ? loop_control : N;
    float float_result = static_simd_reduction(float_data, loop_bound, simd_enabled);
    printf("Float reduction: %f\n", float_result);
    
    /* 3. Double SIMD in noinline function with goto */
    int skip_simd = (N < 500) ? 1 : 0;
    double double_result = noinline_simd_compute(double_data, N, skip_simd);
    printf("Double computation: %f\n", double_result);
    
    /* 4. Mixed OpenMP operations */
    mixed_omp_operations(int_data, N);
    
    /* Additional SIMD loop with ternary operator control */
    int final_sum = 0;
    int simd_len = (use_simt_path) ? 32 : 16;
    
    /* Ternary in condition to create complex GIMPLE */
    #pragma omp simd simdlen(simd_len) reduction(+:final_sum)
    for (int i = 0; i < N; i++) {
        final_sum += (int_data[i] > 50) ? int_data[i] : -int_data[i];
    }
    
    printf("Final SIMD sum: %d\n", final_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* External control function definition */
void external_control(int *flag) {
    *flag = (*flag > 0) ? 0 : 1;
    
    /* Another SIMD loop inside external function */
    int local_data[100];
    int local_sum = 0;
    
    for (int i = 0; i < 100; i++) {
        local_data[i] = i * *flag;
    }
    
    /* SIMD with volatile control */
    volatile int v = *flag;
    if (v) {
        #pragma omp simd simdlen(4) reduction(+:local_sum)
        for (int i = 0; i < 100; i++) {
            local_sum += local_data[i];
        }
    }
}
