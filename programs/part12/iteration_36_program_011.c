/* Test program specifically designed to trigger the uncovered SIMT transformation
   code in GCC's omp-low.cc (lines 2941-2975) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    /* SIMD loop with multiple clauses */
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
        arr[i] = i % 100;
    }
    
    /* Regular parallel region */
    #pragma omp parallel
    {
        #pragma omp for
        for (int i = 0; i < 10; i++) {
            /* Dummy work */
            arr[i % size] += i;
        }
    }
    
    free(arr);
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Parse command-line arguments for runtime control */
    int test_size = 1024;
    int use_simt = 0;
    
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = 1024;
    }
    if (argc > 2) {
        use_simt = atoi(argv[2]) & 1;
    }
    
    /* Set volatile control variables */
    use_simt_path = use_simt;
    loop_control = test_size;
    
    /* Test 1: Integer reduction in main() with conditional SIMD */
    int *int_data = (int*)malloc(test_size * sizeof(int));
    int int_sum = 0;
    
    for (int i = 0; i < test_size; i++) {
        int_data[i] = (i * 3) % 97;
    }
    
    /* Conditional wrapper around SIMD loop - triggers SIMT transformation */
    if (simd_enabled && (use_simt_path || get_random() > 0)) {
        #pragma omp simd simdlen(8) reduction(+:int_sum)
        for (int i = 0; i < test_size; i += 2) {
            int_sum += int_data[i] - int_data[i + 1];
        }
    } else {
        /* Fallback path */
        for (int i = 0; i < test_size; i++) {
            int_sum += int_data[i];
        }
    }
    
    printf("Integer sum: %d\n", int_sum);
    
    /* Test 2: Float reduction in static function */
    float *float_data = (float*)malloc(test_size * sizeof(float));
    for (int i = 0; i < test_size; i++) {
        float_data[i] = (float)i / 10.0f;
    }
    
    float float_sum = static_simd_reduction(float_data, test_size, 
                                           use_simt_path | (test_size > 500));
    printf("Float sum: %.2f\n", float_sum);
    
    /* Test 3: Double reduction in noinline function with goto */
    double *double_data = (double*)malloc(test_size * sizeof(double));
    for (int i = 0; i < test_size; i++) {
        double_data[i] = (double)(i % 50) * 0.25;
    }
    
    double double_sum = noinline_simd_reduction(double_data, test_size, 
                                                use_simt_path);
    printf("Double sum: %.2f\n", double_sum);
    
    /* Test 4: Mixed OpenMP constructs */
    mixed_omp_constructs(test_size / 2);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}

/* Dummy external function implementation */
int get_random(void) {
    static int counter = 0;
    return counter++ % 2;
}
