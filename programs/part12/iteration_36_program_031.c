/* Test program specifically designed to trigger the uncovered SIMT transformation
   in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent optimization */
volatile int simd_enabled = 1;
volatile int use_simt_path = 0;
volatile int loop_control = 100;

/* External function to confuse the optimizer */
extern int get_external_value(void);

/* Static helper function with SIMD loop */
static float static_simd_reduction(float *data, int n, int enable) {
    float sum = 0.0f;
    
    /* Conditional execution to force control flow generation */
    if (enable) {
        /* This SIMD loop should trigger the SIMT transformation */
        #pragma omp simd simdlen(4) reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += data[i];
        }
    } else {
        /* Alternative path without SIMD */
        for (int i = 0; i < n; i++) {
            sum += data[i] * 0.5f;
        }
    }
    
    return sum;
}

/* Noinline function to ensure separate function context */
__attribute__((noinline)) 
double noinline_simd_reduction(double *data, int n, int flag) {
    double result = 0.0;
    int use_simd = flag & 0x1;
    
    /* Complex control flow with goto to interact with artificial labels */
    if (use_simd) {
        goto do_simd;
    } else {
        goto skip_simd;
    }
    
do_simd:
    /* SIMD loop with different data type (double) */
    #pragma omp simd simdlen(8) reduction(+:result) aligned(data:32)
    for (int i = 0; i < n; i += 2) {
        result += data[i] + data[i + 1];
    }
    goto finish;
    
skip_simd:
    /* Non-SIMD path */
    for (int i = 0; i < n; i++) {
        result += data[i];
    }
    
finish:
    return result;
}

/* Function with mixed OpenMP constructs */
void mixed_omp_constructs(int size) {
    int *array = (int*)malloc(size * sizeof(int));
    
    /* Initialize array */
    #pragma omp parallel for simd simdlen(4)
    for (int i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        array[tid % size] += tid;
    }
    
    free(array);
}

/* Main test function */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int loop_size = 1000;
    int use_simt = 0;
    
    if (argc > 1) {
        loop_size = atoi(argv[1]);
        if (loop_size <= 0) loop_size = 100;
    }
    if (argc > 2) {
        use_simt = atoi(argv[2]) & 0x1;
    }
    
    /* Set volatile control variables */
    use_simt_path = use_simt;
    loop_control = loop_size;
    
    /* Test 1: Integer reduction in main() with conditional SIMD */
    int int_sum = 0;
    int *int_data = (int*)malloc(loop_size * sizeof(int));
    
    for (int i = 0; i < loop_size; i++) {
        int_data[i] = (i * 3) % 7;
    }
    
    /* Conditional wrapper around SIMD loop - crucial for triggering the uncovered code */
    if (simd_enabled && (get_external_value() || use_simt_path)) {
        /* This should trigger expand_omp_simt_simd with the conditional wrapper */
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < loop_control; i++) {
            int_sum += int_data[i];
        }
    } else {
        /* Fallback path */
        for (int i = 0; i < loop_size; i++) {
            int_sum += int_data[i];
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Float reduction in static function */
    float *float_data = (float*)malloc(loop_size * sizeof(float));
    for (int i = 0; i < loop_size; i++) {
        float_data[i] = (float)i * 0.1f;
    }
    
    float float_sum = static_simd_reduction(float_data, loop_size, 
                                           simd_enabled && (loop_size > 50));
    printf("Float sum: %.2f\n", float_sum);
    
    /* Test 3: Double reduction in noinline function with goto control flow */
    double *double_data = (double*)malloc(loop_size * sizeof(double));
    for (int i = 0; i < loop_size; i++) {
        double_data[i] = (double)i * 0.05;
    }
    
    double double_sum = noinline_simd_reduction(double_data, loop_size, 
                                               use_simt_path | (loop_size & 0x1));
    printf("Double sum: %.2f\n", double_sum);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(loop_size / 10);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy external function implementation */
int get_external_value(void) {
    static int counter = 0;
    return counter++ % 2;
}
