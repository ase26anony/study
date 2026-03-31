#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to prevent inlining */
extern void process_results(int, float, double);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float* data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional execution around SIMD loop */
    if (enable) {
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

/* Noinline function with goto control flow */
__attribute__((noinline)) 
double noinline_simd_computation(double* array, int size, int flag) {
    double result = 0.0;
    int compute = flag & 0x1;
    
    /* Goto-based control flow to interact with artificial labels */
    if (!compute) {
        goto skip_simd;
    }
    
    /* SIMD loop with double precision */
    #pragma omp simd simdlen(4) reduction(+:result)
    for (int i = 0; i < size; i++) {
        result += sqrt(array[i] + 1.0);
    }
    
    goto finish;
    
skip_simd:
    /* Non-SIMD fallback */
    for (int i = 0; i < size; i++) {
        result += array[i];
    }
    
finish:
    return result;
}

/* Function with mixed OpenMP constructs */
void parallel_and_simd(int* data, int n) {
    int i;
    
    /* Regular parallel for */
    #pragma omp parallel for
    for (i = 0; i < n; i++) {
        data[i] = i * 2;
    }
    
    /* Conditional SIMD with volatile control */
    volatile int do_simd = simd_enabled;
    if (do_simd > 0) {
        int sum = 0;
        
        /* SIMD reduction with varying parameters */
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (i = 0; i < n; i++) {
            sum += data[i] % 17;  /* Complex enough to prevent optimization */
        }
        
        printf("SIMD reduction result: %d\n", sum);
    }
}

/* Main function with command-line control */
int main(int argc, char** argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = 1000;
    int use_simt = 0;
    
    if (argc > 1) {
        loop_size = atoi(argv[1]);
        if (loop_size <= 0) loop_size = 1000;
    }
    if (argc > 2) {
        use_simt = atoi(argv[2]) & 0x1;
    }
    
    /* Set volatile control variables */
    simd_enabled = (loop_size > 500) ? 1 : 0;
    use_simt_path = use_simt;
    
    /* Array 1: Integer reduction in main */
    int* int_data = (int*)malloc(loop_size * sizeof(int));
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = i + 1;
    }
    
    int int_sum = 0;
    
    /* Conditional SIMD with ternary operator */
    (use_simt_path) ? 
    (
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_size; i++) {
            int_sum += int_data[i] * ((i % 3) + 1);
        }
    ) : 
    (
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_size; i++) {
            int_sum += int_data[i];
        }
    );
    
    printf("Integer SIMD reduction: %d\n", int_sum);
    
    /* Array 2: Float reduction in static function */
    float* float_data = (float*)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i / 10.0f;
    }
    
    float float_sum = static_simd_reduction(float_data, loop_size, simd_enabled);
    printf("Float SIMD reduction: %f\n", float_sum);
    
    /* Array 3: Double reduction in noinline function */
    double* double_data = (double*)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = sin((double)i / 100.0);
    }
    
    double double_sum = noinline_simd_computation(double_data, loop_size, use_simt_path);
    printf("Double SIMD computation: %f\n", double_sum);
    
    /* Mixed OpenMP constructs */
    parallel_and_simd(int_data, loop_size);
    
    /* Process results to prevent dead code elimination */
    process_results(int_sum, float_sum, double_sum);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy implementation to satisfy external declaration */
void process_results(int a, float b, double c) {
    printf("Final results: %d, %f, %f\n", a, b, c);
}
