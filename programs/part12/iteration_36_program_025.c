/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;

/* External function to force control flow */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *arr, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional wrapper to force SIMT transformation */
    if (flag > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += arr[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum -= arr[i];
        }
    }
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *arr, int n, int condition) {
    double result = 0.0;
    
    /* Goto statements to interact with artificial labels */
    if (condition) {
        goto simd_block;
    } else {
        goto skip_simd;
    }
    
simd_block:
    /* SIMD loop with conditional execution */
    if (use_simt_path) {
        #pragma omp simd simdlen(16) reduction(+:result)
        for (int i = 0; i < n; i += 2) {
            result += arr[i] * arr[i+1];
        }
    }
    goto end;
    
skip_simd:
    /* Alternative path without SIMD */
    for (int i = 0; i < n; i++) {
        result += arr[i];
    }
    
end:
    return result;
}

/* Main function with multiple SIMD constructs */
int main(int argc, char **argv) {
    int N = 1024;
    if (argc > 1) N = atoi(argv[1]);
    
    /* Parse command-line arguments for control flow */
    int simd_flag = (argc > 2) ? atoi(argv[2]) : 1;
    use_simt_path = (argc > 3) ? atoi(argv[3]) : 0;
    
    /* Array initializations */
    int *int_arr = (int*)malloc(N * sizeof(int));
    float *float_arr = (float*)malloc(N * sizeof(float));
    double *double_arr = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        int_arr[i] = i % 100;
        float_arr[i] = i * 0.5f;
        double_arr[i] = i * 0.25;
    }
    
    /* 1. Integer SIMD reduction in main with conditional */
    int int_sum = 0;
    if (simd_enabled && (simd_flag & 1)) {
        /* This should trigger SIMT transformation */
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < N; i++) {
            int_sum += int_arr[i];
        }
    } else {
        /* Fallback without SIMD */
        for (int i = 0; i < N; i++) {
            int_sum += int_arr[i];
        }
    }
    
    /* 2. Float SIMD reduction in static function */
    float float_sum = static_simd_reduction(float_arr, N, simd_flag);
    
    /* 3. Double SIMD reduction in noinline function with goto */
    double double_sum = noinline_simd_reduction(double_arr, N, use_simt_path);
    
    /* 4. Additional mixed OpenMP construct to activate infrastructure */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < N/2; i++) {
        int_arr[i] *= 2;
    }
    
    /* 5. Ternary operator controlled SIMD loop */
    int alt_sum = 0;
    (use_simt_path) ? 
    (
        #pragma omp simd simdlen(2) reduction(+:alt_sum)
        for (int i = 0; i < N; i += 4) {
            alt_sum += int_arr[i];
        }
    ) : (
        #pragma omp simd simdlen(8) reduction(+:alt_sum)
        for (int i = 0; i < N; i += 2) {
            alt_sum -= int_arr[i];
        }
    );
    
    printf("Results: int=%d, float=%.2f, double=%.2f, alt=%d\n",
           int_sum, float_sum, double_sum, alt_sum);
    
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}
