/* Test program specifically designed to trigger the uncovered SIMT transformation
   code in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional wrapper to force SIMT transformation */
    if (flag > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
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
    
    /* Complex control flow with goto to interact with artificial labels */
    if (skip) {
        goto skip_simd;
    }
    
    /* This SIMD loop should trigger the uncovered code */
    if (simd_enabled) {
        #pragma omp simd simdlen(4) reduction(+:result)
        for (int i = 0; i < n; i += 2) {  /* Non-unit stride to stress transformation */
            result += data[i];
        }
    }
    
skip_simd:
    /* Another SIMD loop with different parameters */
    if (!skip) {
        #pragma omp simd simdlen(8) reduction(+:result)
        for (int i = 1; i < n; i += 2) {
            result += data[i] * 0.5;
        }
    }
    
    return result;
}

/* Helper function with conditional SIMD execution */
int helper_simd_reduction(int *arr, int size, volatile int *flag) {
    int total = 0;
    
    /* Ternary operator to force conditional wrapper generation */
    (*flag > 0) ? 
    (
        #pragma omp simd simdlen(16) reduction(+:total)
        for (int i = 0; i < size; i++) {
            total += arr[i];
        }
    ) :
    (
        #pragma omp simd simdlen(8) reduction(+:total)
        for (int i = 0; i < size; i++) {
            total -= arr[i];
        }
    );
    
    return total;
}

/* Mixed OpenMP constructs to activate full OMP infrastructure */
void parallel_region(int *data, int n) {
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < n; i++) {
        data[i] *= 2;
    }
}

int main(int argc, char **argv) {
    const int N = 1024;
    int *int_data = (int*)malloc(N * sizeof(int));
    float *float_data = (float*)malloc(N * sizeof(float));
    double *double_data = (double*)malloc(N * sizeof(double));
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        int_data[i] = i % 100;
        float_data[i] = (float)(i % 100) * 0.5f;
        double_data[i] = (double)(i % 100) * 0.25;
    }
    
    /* Parse command-line arguments for runtime control */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 100;
    use_simt_path = (argc > 2) ? atoi(argv[2]) : 0;
    
    /* 1. Integer SIMD reduction in helper function with volatile control */
    volatile int control_flag = use_simt_path;
    int int_result = helper_simd_reduction(int_data, loop_count, &control_flag);
    printf("Integer reduction result: %d\n", int_result);
    
    /* 2. Float SIMD reduction in static function with runtime bounds */
    float float_result = static_simd_reduction(float_data, 
                                              (loop_count > N) ? N : loop_count,
                                              use_simt_path);
    printf("Float reduction result: %f\n", float_result);
    
    /* 3. Double SIMD reduction in noinline function with goto */
    double double_result = noinline_simd_reduction(double_data, 
                                                   loop_count,
                                                   use_simt_path == 0);
    printf("Double reduction result: %lf\n", double_result);
    
    /* 4. Mixed OpenMP parallel for simd */
    parallel_region(int_data, (loop_count < N) ? loop_count : N);
    
    /* Verify some results to prevent dead code elimination */
    int verify_sum = 0;
    #pragma omp simd simdlen(8) reduction(+:verify_sum)
    for (int i = 0; i < (loop_count % 100); i++) {
        verify_sum += int_data[i];
    }
    printf("Verification sum: %d\n", verify_sum);
    
    /* Additional conditional SIMD with complex control flow */
    {
        int a = 0, b = 0;
        
        /* Nested conditions to stress the transformation */
        if (simd_enabled) {
            if (use_simt_path) {
                #pragma omp simd simdlen(4) reduction(+:a)
                for (int i = 0; i < loop_count; i++) {
                    a += i;
                }
            } else {
                #pragma omp simd simdlen(8) reduction(+:b)
                for (int i = 0; i < loop_count; i++) {
                    b += i * 2;
                }
            }
        }
        
        /* Final SIMD with different data type */
        long long ll_sum = 0;
        #pragma omp simd simdlen(2) reduction(+:ll_sum)
        for (int i = 0; i < loop_count; i++) {
            ll_sum += (long long)i * i;
        }
        printf("Long long sum: %lld\n", ll_sum);
    }
    
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
