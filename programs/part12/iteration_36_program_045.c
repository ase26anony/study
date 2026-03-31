/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
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
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional execution to force gbind creation */
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
double noinline_simd_reduction(double *data, int n, int flag) {
    double result = 0.0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (flag > 0) {
        goto compute_simd;
    } else {
        goto skip_simd;
    }
    
compute_simd:
    /* SIMD loop with varying parameters */
    #pragma omp simd simdlen(16) reduction(+:result) aligned(data:64)
    for (int i = 0; i < n; i += 2) {
        result += data[i] * data[i + 1];
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

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *arr = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
    }
    
    /* Conditional SIMD reduction */
    int sum = 0;
    
    /* Ternary operator to force conditional wrapper */
    (use_simt_path) ? 
    (
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < size; i++) {
            sum += arr[i] / (i + 1);
        }
    ) :
    (
        #pragma omp simd simdlen(16) reduction(+:sum)
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
    );
    
    printf("Mixed constructs sum: %d\n", sum);
    free(arr);
}

/* Main function with diverse SIMD contexts */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = (argc > 1) ? atoi(argv[1]) : 1000;
    int simd_flag = (argc > 2) ? atoi(argv[2]) : 1;
    
    /* Force runtime evaluation */
    use_simt_path = get_random() % 2;
    loop_control = loop_size;
    
    /* 1. Integer SIMD reduction in main() */
    int int_sum = 0;
    int *int_data = (int*)malloc(loop_control * sizeof(int));
    
    for (int i = 0; i < loop_control; i++) {
        int_data[i] = i + 1;
    }
    
    /* Nested conditional to force SIMT transformation */
    if (simd_enabled) {
        if (use_simt_path) {
            #pragma omp simd simdlen(8) reduction(+:int_sum)
            for (int i = 0; i < loop_control; i++) {
                int_sum += int_data[i];
            }
        } else {
            #pragma omp simd simdlen(4) reduction(+:int_sum)
            for (int i = 0; i < loop_control; i++) {
                int_sum += int_data[i] * 2;
            }
        }
    }
    
    printf("Integer reduction: %d\n", int_sum);
    
    /* 2. Float SIMD in static function */
    float *float_data = (float*)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i / 10.0f;
    }
    
    float float_result = static_simd_reduction(float_data, loop_size, simd_flag);
    printf("Float reduction: %.2f\n", float_result);
    
    /* 3. Double SIMD in noinline function with goto */
    double *double_data = (double*)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = (double)i / 5.0;
    }
    
    double double_result = noinline_simd_reduction(double_data, loop_size, use_simt_path);
    printf("Double reduction: %.2f\n", double_result);
    
    /* 4. Mixed OpenMP constructs */
    mixed_omp_constructs(loop_size / 2);
    
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
