/* Test program to trigger SIMT transformation in GCC's omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int loop_control = 1000;
volatile int use_simt_path = 0;

/* Static helper function with SIMD loop */
static float static_simd_reduction(float* data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional wrapper to force SIMT transformation */
    if (enable) {
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
double noinline_simd_reduction(double* data, int n, int skip_first) {
    double result = 0.0;
    int i = 0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (skip_first) {
        goto skip_simd;
    }
    
    /* First SIMD loop - may be skipped */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (i = 0; i < n/2; i++) {
        result += data[i];
    }
    
skip_simd:
    /* Second SIMD loop - always executed */
    #pragma omp simd simdlen(8) reduction(+:result)
    for (; i < n; i++) {
        result += data[i] * 3.0;
    }
    
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int* array = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Conditional SIMD reduction in main computation */
    int sum = 0;
    
    /* Ternary operator to force conditional wrapper */
    (use_simt_path) ? 
    (
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (int i = 0; i < size; i += 2) {
            sum += array[i];
        }
    ) : (
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 1; i < size; i += 2) {
            sum += array[i];
        }
    );
    
    printf("Mixed OMP sum: %d\n", sum);
    free(array);
}

/* Main function with diverse SIMD constructs */
int main(int argc, char** argv) {
    /* Parse command-line arguments for runtime control */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int use_simt = (argc > 2) ? atoi(argv[2]) : 0;
    
    /* Set volatile control variables */
    simd_enabled = (loop_count > 0);
    use_simt_path = use_simt;
    
    /* 1. Integer SIMD reduction in main() with conditional */
    int int_sum = 0;
    int* int_data = (int*)malloc(loop_count * sizeof(int));
    
    for (int i = 0; i < loop_count; i++) {
        int_data[i] = (i % 7) + 1;
    }
    
    /* Complex condition to force SIMT transformation */
    if (simd_enabled && (loop_count % 3 == 0 || use_simt)) {
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_data[i];
        }
    } else {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i += 2) {
            int_sum += int_data[i] * 2;
        }
    }
    
    printf("Integer reduction sum: %d\n", int_sum);
    free(int_data);
    
    /* 2. Float SIMD reduction in static function */
    int float_size = loop_count / 2;
    float* float_data = (float*)malloc(float_size * sizeof(float));
    
    for (int i = 0; i < float_size; i++) {
        float_data[i] = (i % 11) * 0.5f;
    }
    
    float float_sum = static_simd_reduction(float_data, float_size, use_simt);
    printf("Float reduction sum: %.2f\n", float_sum);
    free(float_data);
    
    /* 3. Double SIMD reduction in noinline function with goto */
    int double_size = loop_count / 3;
    double* double_data = (double*)malloc(double_size * sizeof(double));
    
    for (int i = 0; i < double_size; i++) {
        double_data[i] = (i % 13) * 0.25;
    }
    
    double double_sum = noinline_simd_reduction(double_data, double_size, 
                                                (loop_count % 5 == 0));
    printf("Double reduction sum: %.2f\n", double_sum);
    free(double_data);
    
    /* 4. Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 4);
    
    /* Additional nested conditional SIMD to stress the transformation */
    {
        volatile int inner_control = loop_control;
        int test_sum = 0;
        
        /* Deeply nested condition */
        if (inner_control > 500) {
            if (use_simt_path) {
                #pragma omp simd simdlen(2) reduction(+:test_sum)
                for (int i = 0; i < 100; i++) {
                    test_sum += i * (inner_control % 17);
                }
            }
        }
        
        printf("Nested SIMD test sum: %d\n", test_sum);
    }
    
    return 0;
}
