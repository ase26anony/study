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
    
    /* Conditional wrapper - forces compiler to generate control flow */
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

/* Noinline function with goto-based control flow */
__attribute__((noinline)) 
double noinline_simd_compute(double *arr, int size, int skip_first) {
    double result = 0.0;
    int i = 0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (skip_first) {
        goto skip_simd;
    }
    
    /* This SIMD loop should trigger the SIMT transformation */
    #pragma omp simd simdlen(16) reduction(+:result) aligned(arr:64)
    for (i = 0; i < size; i++) {
        result += arr[i] * (i % 2 ? -1.0 : 1.0);
    }
    
    goto done;
    
skip_simd:
    /* Alternative computation without SIMD */
    for (i = 0; i < size; i++) {
        result += arr[i];
    }
    
done:
    return result;
}

/* Main function with multiple SIMD constructs */
int main(int argc, char **argv) {
    const int N = 1024;
    int *int_data = (int*)aligned_alloc(64, N * sizeof(int));
    float *float_data = (float*)aligned_alloc(64, N * sizeof(float));
    double *double_data = (double*)aligned_alloc(64, N * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        int_data[i] = i + 1;
        float_data[i] = (i + 1) * 0.1f;
        double_data[i] = (i + 1) * 0.01;
    }
    
    /* Parse command line for runtime control */
    int use_simd = 1;
    int simd_len = 8;
    if (argc > 1) {
        use_simd = atoi(argv[1]);
        if (argc > 2) {
            simd_len = atoi(argv[2]);
        }
    }
    
    /* Result accumulators */
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* 1. Integer SIMD reduction in main() with conditional execution */
    /* Complex condition to force control flow generation */
    int enable_int_simd = (use_simd > 0) && (simd_enabled || (get_random() % 2));
    
    if (enable_int_simd) {
        /* This should trigger expand_omp_simt_simd */
        #pragma omp simd simdlen(simd_len) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i % N] * (i % 3);
        }
    } else {
        /* Fallback without SIMD */
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i % N];
        }
    }
    
    /* 2. Float SIMD in static function with varying parameters */
    int float_loop_bound = (argc > 1) ? atoi(argv[1]) % N : N;
    if (float_loop_bound < 100) float_loop_bound = 100;
    
    /* Ternary operator to force conditional wrapper */
    int float_enable = (use_simd % 2) ? simd_enabled : use_simt_path;
    float_sum = static_simd_reduction(float_data, float_loop_bound, float_enable);
    
    /* 3. Double SIMD in noinline function with goto control flow */
    int skip_double = (argc > 1 && strcmp(argv[1], "skip") == 0);
    double_sum = noinline_simd_compute(double_data, N, skip_double);
    
    /* 4. Additional mixed OpenMP constructs to activate full OMP lowering */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < N/2; i++) {
        float_data[i] = float_data[i] * 2.0f;
    }
    
    /* 5. Nested conditional SIMD with volatile control */
    volatile int volatile_ctrl = 1;
    int extra_sum = 0;
    
    if (volatile_ctrl && use_simd) {
        #pragma omp simd simdlen(16) reduction(+:extra_sum)
        for (int i = 0; i < N; i += 2) {
            extra_sum += int_data[i] - int_data[i+1];
        }
    }
    
    /* Print results to prevent elimination */
    printf("Results:\n");
    printf("  Integer SIMD sum: %d\n", int_sum);
    printf("  Float SIMD sum: %.4f\n", float_sum);
    printf("  Double SIMD sum: %.4f\n", double_sum);
    printf("  Extra sum: %d\n", extra_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy external function */
int get_random(void) {
    return rand();
}
