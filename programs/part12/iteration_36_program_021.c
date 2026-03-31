/* Test program specifically designed to trigger the uncovered SIMT transformation
   code in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;

/* External function to force runtime evaluation */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *arr, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional execution to force SIMT wrapper generation */
    if (flag > 0) {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += arr[i];
        }
    } else {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum -= arr[i];
        }
    }
    return sum;
}

/* Noinline function with goto-based control flow */
__attribute__((noinline)) 
double noinline_simd_computation(double *data, int size, int skip) {
    double result = 0.0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (skip) {
        goto skip_simd;
    }
    
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(2) reduction(+:result) aligned(data:32)
    for (int i = 0; i < size; i++) {
        result += data[i] * 2.0;
    }
    
    goto end_computation;
    
skip_simd:
    /* Alternative path without SIMD */
    for (int i = 0; i < size; i++) {
        result += data[i];
    }
    
end_computation:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int n) {
    int *array = (int*)malloc(n * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd
    for (int i = 0; i < n; i++) {
        array[i] = i % 100;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Threads: %d\n", omp_get_num_threads());
        }
    }
    
    free(array);
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_count = 1000;
    int use_simt = 0;
    
    if (argc > 1) loop_count = atoi(argv[1]);
    if (argc > 2) use_simt = atoi(argv[2]);
    
    /* Force volatile flag evaluation */
    use_simt_path = use_simt;
    
    /* Test 1: Integer SIMD reduction in main() with conditional */
    int int_sum = 0;
    int *int_arr = (int*)malloc(loop_count * sizeof(int));
    
    for (int i = 0; i < loop_count; i++) {
        int_arr[i] = i + 1;
    }
    
    /* Conditional wrapper that should trigger SIMT transformation */
    if (simd_enabled && (use_simt_path || get_random() > 0)) {
        #pragma omp simd simdlen(16) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_arr[i];
        }
    } else {
        /* Fallback path */
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_arr[i];
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Float SIMD in static function */
    float *float_arr = (float*)malloc(loop_count * sizeof(float));
    for (int i = 0; i < loop_count; i++) {
        float_arr[i] = (float)i / 10.0f;
    }
    
    float float_result = static_simd_reduction(float_arr, loop_count, 
                                              simd_enabled);
    printf("Float reduction: %f\n", float_result);
    
    /* Test 3: Double SIMD in noinline function with goto */
    double *double_arr = (double*)malloc(loop_count * sizeof(double));
    for (int i = 0; i < loop_count; i++) {
        double_arr[i] = (double)i / 5.0;
    }
    
    /* Ternary operator to force conditional evaluation */
    int skip_flag = (loop_count < 500) ? 1 : 0;
    double double_result = noinline_simd_computation(double_arr, loop_count, 
                                                     skip_flag);
    printf("Double computation: %lf\n", double_result);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 10);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    return rand() % 100;
}
