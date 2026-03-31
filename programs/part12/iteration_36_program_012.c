/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc, lines 2941-2975. This creates conditional wrappers with
   artificial labels and bind expressions for SIMD loops targeting SIMT execution. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional execution to force SIMT wrapper generation */
    if (flag > 0) {
        #pragma omp simd reduction(+:sum) simdlen(8)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd reduction(+:sum) simdlen(4)
        for (int i = 0; i < n; i++) {
            sum += data[i] * 2.0f;
        }
    }
    
    return sum;
}

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int skip) {
    double result = 0.0;
    
    /* Goto statements to interact with artificial labels */
    if (skip) {
        goto skip_simd;
    }
    
    /* This SIMD loop should be wrapped in SIMT conditional */
    #pragma omp simd reduction(+:result) simdlen(16)
    for (int i = 0; i < n; i++) {
        result += data[i] * (i % 2 ? -1.0 : 1.0);
    }
    
    goto finish;
    
skip_simd:
    /* Alternative path without SIMD */
    for (int i = 0; i < n; i++) {
        result += data[i];
    }
    
finish:
    return result;
}

/* Main function with multiple SIMD constructs */
int main(int argc, char **argv) {
    int N = 1024;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1024;
    }
    
    /* Parse command-line arguments for control flow */
    int use_simt = (argc > 2) ? atoi(argv[2]) : 0;
    int mixed_mode = (argc > 3) ? atoi(argv[3]) : 1;
    
    /* Allocate and initialize arrays */
    int *int_data = (int*)malloc(N * sizeof(int));
    float *float_data = (float*)malloc(N * sizeof(float));
    double *double_data = (double*)malloc(N * sizeof(double));
    
    for (int i = 0; i < N; i++) {
        int_data[i] = i + 1;
        float_data[i] = (i + 1) * 0.5f;
        double_data[i] = (i + 1) * 0.25;
    }
    
    int int_sum = 0;
    float float_sum = 0.0f;
    double double_sum = 0.0;
    
    /* SIMD loop 1: Integer reduction in main() with runtime condition */
    /* This should trigger the SIMT transformation with conditional wrapper */
    if (simd_enabled && (use_simt || mixed_mode)) {
        #pragma omp simd reduction(+:int_sum) simdlen(8)
        for (int i = 0; i < N; i++) {
            int_sum += int_data[i];
        }
    } else {
        /* Non-SIMD fallback */
        for (int i = 0; i < N; i++) {
            int_sum += int_data[i];
        }
    }
    
    /* SIMD loop 2: Float reduction in static function */
    /* Nested conditions to force complex control flow */
    int flag = (use_simt_path > 0) ? 1 : 0;
    if (mixed_mode) {
        flag = get_random() % 2;
    }
    
    float_sum = static_simd_reduction(float_data, N, flag);
    
    /* SIMD loop 3: Double reduction in noinline function with goto */
    /* Ternary operator to create conditional execution */
    int skip = (use_simt == 2) ? 1 : 0;
    double_sum = noinline_simd_reduction(double_data, N, skip);
    
    /* Additional mixed OpenMP constructs to activate full OMP lowering */
    #pragma omp parallel for simd if(N > 512)
    for (int i = 0; i < N; i++) {
        int_data[i] *= 2;
    }
    
    /* Print results to prevent elimination */
    printf("Results:\n");
    printf("  Integer sum: %d\n", int_sum);
    printf("  Float sum: %.2f\n", float_sum);
    printf("  Double sum: %.2f\n", double_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    return rand();
}
