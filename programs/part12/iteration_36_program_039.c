/* Test program specifically designed to trigger the uncovered SIMT transformation
   code in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 1000;

/* External function to force control flow */
extern int get_random(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional execution to force SIMT wrapper generation */
    if (enable) {
        #pragma omp simd simdlen(8) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
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
double noinline_simd_reduction(double *data, int n, int flag) {
    double result = 0.0;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (flag > 0) {
        goto compute_simd;
    } else {
        goto skip_simd;
    }
    
compute_simd:
    /* SIMD loop with conditional execution wrapper */
    if (simd_enabled) {
        #pragma omp simd simdlen(4) reduction(+:result)
        for (int i = 0; i < n; i += 2) {  /* Non-unit stride to stress SIMD */
            result += data[i] * 2.0;
        }
    }
    goto finish;
    
skip_simd:
    result = -1.0;
    
finish:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *array = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(16)
    for (int i = 0; i < size; i++) {
        array[i] = i % 100;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        array[tid % size] += tid;
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
    
    /* Force volatile writes to prevent constant propagation */
    simd_enabled = (get_random() % 2) || use_simt;
    use_simt_path = use_simt;
    loop_control = loop_count;
    
    /* Test 1: Integer reduction in main() with conditional SIMD */
    int *int_data = (int*)malloc(loop_count * sizeof(int));
    int int_sum = 0;
    
    for (int i = 0; i < loop_count; i++) {
        int_data[i] = i + 1;
    }
    
    /* Conditional wrapper that should trigger SIMT transformation */
    if (simd_enabled || use_simt_path) {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_data[i];
        }
    } else {
        /* Fallback path */
        for (int i = 0; i < loop_count; i++) {
            int_sum += int_data[i];
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Float reduction in static function */
    float *float_data = (float*)malloc(loop_count * sizeof(float));
    for (int i = 0; i < loop_count; i++) {
        float_data[i] = (float)i * 0.5f;
    }
    
    float float_sum = static_simd_reduction(float_data, loop_count, 
                                           simd_enabled && (loop_count > 500));
    printf("Float sum: %f\n", float_sum);
    
    /* Test 3: Double reduction with noinline function and goto */
    double *double_data = (double*)malloc(loop_count * sizeof(double));
    for (int i = 0; i < loop_count; i++) {
        double_data[i] = (double)i * 0.25;
    }
    
    double double_sum = noinline_simd_reduction(double_data, loop_count, 
                                                simd_enabled);
    printf("Double result: %lf\n", double_sum);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(loop_count / 10);
    
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
