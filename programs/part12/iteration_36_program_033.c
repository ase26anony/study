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
static float static_simd_reduction(float *data, int n, int flag) {
    float sum = 0.0f;
    
    /* Conditional wrapper around SIMD loop - forces gbind creation */
    if (flag > 0) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum -= data[i];
        }
    }
    return sum;
}

/* Noinline function with goto-based control flow */
__attribute__((noinline)) 
double noinline_simd_loop(double *arr, int size, int skip_first) {
    double result = 0.0;
    int i = 0;
    
    /* Goto to interact with artificial label creation */
    if (skip_first) {
        goto skip_simd;
    }
    
    /* SIMD loop with multiple clauses */
    #pragma omp simd simdlen(4) reduction(+:result) linear(i:1)
    for (i = 0; i < size; i++) {
        result += arr[i] * (i + 1);
    }
    
skip_simd:
    /* Another SIMD loop that might be executed conditionally */
    if (use_simt_path) {
        #pragma omp simd simdlen(8) reduction(+:result)
        for (int j = 0; j < size/2; j++) {
            result -= arr[j];
        }
    }
    
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int n) {
    int *data = (int*)malloc(n * sizeof(int));
    
    /* Initialize data */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < n; i++) {
        data[i] = i % 100;
    }
    
    /* Conditional SIMD reduction - triggers SIMT transformation */
    int sum = 0;
    int flag = get_random() % 2;
    
    /* Complex condition to force runtime evaluation */
    if ((simd_enabled && flag) || (use_simt_path && n > 100)) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum -= data[i];
        }
    }
    
    printf("Mixed constructs sum: %d\n", sum);
    free(data);
}

int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_count = 1000;
    if (argc > 1) loop_count = atoi(argv[1]);
    if (argc > 2) use_simt_path = atoi(argv[2]);
    
    /* Test 1: Integer SIMD reduction in main with volatile control */
    int *int_data = (int*)malloc(loop_count * sizeof(int));
    for (int i = 0; i < loop_count; i++) {
        int_data[i] = i;
    }
    
    int int_sum = 0;
    /* Ternary operator to force conditional SIMD execution */
    (simd_enabled > 0) ? 
    (
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_data[i];
        }
    ) : 
    (
        #pragma omp simd simdlen(4) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum -= int_data[i];
        }
    );
    
    printf("Integer sum: %d\n", int_sum);
    free(int_data);
    
    /* Test 2: Float SIMD in static function */
    float *float_data = (float*)malloc(loop_count * sizeof(float));
    for (int i = 0; i < loop_count; i++) {
        float_data[i] = i * 0.5f;
    }
    
    float float_sum = static_simd_reduction(float_data, loop_count, use_simt_path);
    printf("Float sum: %f\n", float_sum);
    free(float_data);
    
    /* Test 3: Double SIMD in noinline function with goto */
    double *double_data = (double*)malloc(loop_count * sizeof(double));
    for (int i = 0; i < loop_count; i++) {
        double_data[i] = i * 0.25;
    }
    
    double double_sum = noinline_simd_loop(double_data, loop_count, use_simt_path);
    printf("Double sum: %f\n", double_sum);
    free(double_data);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 2);
    
    return 0;
}

/* Dummy implementation of external function */
int get_random(void) {
    return rand();
}
