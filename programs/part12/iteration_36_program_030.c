/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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

/* Noinline function with goto to interact with artificial labels */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int condition) {
    double product = 1.0;
    int i;
    
    /* Complex control flow with goto to force label creation */
    if (condition) {
        goto skip_simd;
    }
    
    /* This SIMD loop should trigger the uncovered block */
    #pragma omp simd simdlen(4) reduction(*:product) linear(i:1)
    for (i = 0; i < n; i++) {
        product *= data[i] + 1.0;
    }
    
    goto end;
    
skip_simd:
    /* Alternative path without SIMD */
    for (i = 0; i < n; i++) {
        product *= data[i];
    }
    
end:
    return product;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *arr = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(8)
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        #pragma omp single
        {
            printf("Thread %d initialized array\n", omp_get_thread_num());
        }
    }
    
    free(arr);
}

/* Main test function */
int main(int argc, char **argv) {
    const int N = 1024;
    int int_result = 0;
    float float_result = 0.0f;
    double double_result = 0.0;
    
    /* Parse command line for runtime control */
    int loop_count = (argc > 1) ? atoi(argv[1]) : N;
    use_simt_path = (argc > 2) ? atoi(argv[2]) : 0;
    
    /* Allocate and initialize arrays */
    int *int_arr = (int*)malloc(loop_count * sizeof(int));
    float *float_arr = (float*)malloc(loop_count * sizeof(float));
    double *double_arr = (double*)malloc(loop_count * sizeof(double));
    
    for (int i = 0; i < loop_count; i++) {
        int_arr[i] = (i % 100) + 1;
        float_arr[i] = (i % 100) * 0.1f;
        double_arr[i] = (i % 100) * 0.01;
    }
    
    /* Test 1: Integer SIMD reduction in main with volatile condition */
    if (simd_enabled) {
        /* This should trigger the SIMT transformation */
        #pragma omp simd simdlen(16) reduction(+:int_result) \
                if(use_simt_path)  /* if clause to force conditional */
        for (int i = 0; i < loop_count; i++) {
            int_result += int_arr[i];
        }
    }
    
    /* Test 2: Float SIMD in static function with ternary condition */
    float_result = static_simd_reduction(float_arr, 
                                        (loop_count > 100) ? loop_count : 100,
                                        get_random() % 2);
    
    /* Test 3: Double SIMD in noinline function with goto */
    double_result = noinline_simd_reduction(double_arr, 
                                           loop_count, 
                                           use_simt_path);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 2);
    
    /* Print results to prevent elimination */
    printf("Results:\n");
    printf("  Integer reduction: %d\n", int_result);
    printf("  Float reduction: %f\n", float_result);
    printf("  Double reduction: %f\n", double_result);
    printf("  SIMD enabled flag: %d\n", simd_enabled);
    printf("  Use SIMT path: %d\n", use_simt_path);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    return rand();
}
