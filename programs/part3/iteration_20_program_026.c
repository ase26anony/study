/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
 * 
 * Compilation (for NVIDIA offloading):
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -o test_simt test_simt_lowering.c
 * 
 * For coverage analysis with gcov:
 *   gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -ftest-coverage -fprofile-arcs -o test_simt_coverage test_simt_lowering.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void target_region_simple(float *arr, int size) {
    /* Simple vector scaling - likely vectorizable/SIMD-able */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size])
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * 3.14f + 1.0f;
    }
}

__attribute__((noinline))
void target_region_conditional(float *data, int size, float threshold) {
    /* Conditional update inside loop - creates more complex GIMPLE */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: threshold)
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_region_nested_control(float *a, float *b, float *c, int size) {
    /* Multiple arrays with arithmetic - encourages SIMT transformation */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(tofrom: c[0:size])
    for (int i = 0; i < size; ++i) {
        float temp = a[i] + b[i];
        if (temp > 0.0f) {
            c[i] = temp * c[i];
        } else {
            c[i] = temp / (c[i] + 1.0f);
        }
    }
}

/* Helper function to initialize arrays */
void init_array(float *arr, int size, float base_value) {
    for (int i = 0; i < size; ++i) {
        arr[i] = base_value + i * 0.1f;
    }
}

/* Verification function to ensure computation isn't optimized away */
float verify_sum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    int iterations = 2;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Allocate and initialize test data */
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing SIMT lowering with test_case=%d, iterations=%d\n", 
           test_case, iterations);
    
    /* Loop over multiple iterations to increase coverage chances */
    for (int iter = 0; iter < iterations; ++iter) {
        /* Vary the initialization slightly each iteration */
        init_array(array1, N, 100.0f + iter * 50.0f);
        init_array(array2, N, 200.0f + iter * 30.0f);
        init_array(array3, N, 300.0f + iter * 20.0f);
        
        switch (test_case) {
            case 1:
                /* Simple SIMD loop with explicit simd clause */
                target_region_simple(array1, N);
                printf("  Iter %d, Case 1 - Simple SIMD: sum = %f\n", 
                       iter, verify_sum(array1, N));
                break;
                
            case 2:
                /* Conditional loop - more complex control flow */
                target_region_conditional(array2, N, THRESHOLD + iter * 10.0f);
                printf("  Iter %d, Case 2 - Conditional: sum = %f\n", 
                       iter, verify_sum(array2, N));
                break;
                
            case 3:
                /* Multiple arrays with nested control flow */
                target_region_nested_control(array1, array2, array3, N);
                printf("  Iter %d, Case 3 - Nested control: sum = %f\n", 
                       iter, verify_sum(array3, N));
                break;
                
            default:
                /* Run all three to maximize coverage */
                target_region_simple(array1, N);
                target_region_conditional(array2, N, THRESHOLD);
                target_region_nested_control(array1, array2, array3, N);
                printf("  Iter %d, All cases: sums = %f, %f, %f\n", 
                       iter, verify_sum(array1, N), 
                       verify_sum(array2, N), verify_sum(array3, N));
                break;
        }
        
        /* Also test with different sizes to trigger different expansions */
        if (iter % 2 == 0) {
            /* Test with half size */
            int half_n = N / 2;
            target_region_simple(array1, half_n);
            printf("  Iter %d, Half-size test: sum = %f\n", 
                   iter, verify_sum(array1, half_n));
        }
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    printf("Test completed successfully.\n");
    return 0;
}
