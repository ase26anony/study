/* test_omp_simt_lowering.c
 * Designed to trigger SIMT transformation in omp-low.cc
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fno-inline -fdump-tree-omplower -o test_omp_simt test_omp_simt_lowering.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline, optimize("no-inline")))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        /* Simple vectorizable operation with potential SIMT transformation */
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline, optimize("no-inline")))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        /* Conditional logic inside loop - creates more complex GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline, optimize("no-inline")))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) \
        map(from: c[0:size]) \
        num_teams(16) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        /* Multiple operations with temporary variables */
        float temp = a[i] + b[i];
        if (temp > 0.0f) {
            c[i] = logf(fabsf(temp) + 1.0f);
        } else {
            c[i] = -logf(fabsf(temp) + 1.0f);
        }
    }
}

/* Function with varying loop bounds based on runtime */
__attribute__((noinline, optimize("no-inline")))
void target_simt_variable_bound(float *arr, int start, int end, int step) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[start:end-start]) \
        num_teams(2) thread_limit(32)
    for (int i = start; i < end; i += step) {
        /* Complex enough for SIMT but still vectorizable */
        arr[i] = sinf(arr[i]) * cosf(arr[i]);
    }
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    for (int i = 0; i < N; i++) {
        array1[i] = (float)i;
        array2[i] = (float)(N - i);
        array3[i] = (float)(i % 100);
        result[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute multiple target regions to increase coverage */
    printf("Running test case %d\n", test_case);
    
    switch (test_case) {
        case 1:
            /* Basic SIMD clause - most likely to trigger SIMT */
            target_simt_vector_scale(array1, N, 2.5f);
            break;
            
        case 2:
            /* Conditional logic in loop */
            target_simt_conditional_update(array2, N, THRESHOLD);
            break;
            
        case 3:
            /* Multiple arrays with nested control flow */
            target_simt_nested_control(array1, array2, result, N);
            break;
            
        case 4:
            /* Variable loop bounds */
            target_simt_variable_bound(array3, 0, N, 1);
            target_simt_variable_bound(array3, 1, N, 2);
            break;
            
        default:
            /* Run all tests */
            target_simt_vector_scale(array1, N, 1.5f);
            target_simt_conditional_update(array2, N, THRESHOLD / 2.0f);
            target_simt_nested_control(array1, array2, result, N);
            target_simt_variable_bound(array3, 0, N/2, 1);
            break;
    }
    
    /* Verify computation (prevents dead code elimination) */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        checksum += array1[i] + array2[i] + array3[i] + result[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}
