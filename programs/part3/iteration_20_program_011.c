/* Test program for OpenMP SIMT transformation coverage */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void simple_vector_multiply(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor + 1.0f;
    }
}

__attribute__((noinline))
void conditional_vector_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void nested_loop_computation(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        collapse(2) num_teams(16)
    for (int i = 0; i < size/2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            c[idx] = a[idx] + b[idx] * 2.0f;
            /* Additional computation to create more complex GIMPLE */
            c[idx] = c[idx] + sinf((float)idx * 0.01f);
        }
    }
}

__attribute__((noinline))
void mixed_simd_nosimd(float *arr1, float *arr2, int size, int use_simd) {
    if (use_simd) {
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: arr1[0:size], arr2[0:size])
        for (int i = 0; i < size; ++i) {
            arr1[i] = arr2[i] * 3.14f;
            arr2[i] = arr1[i] / 2.0f;
        }
    } else {
        #pragma omp target teams distribute parallel for \
            map(tofrom: arr1[0:size], arr2[0:size])
        for (int i = 0; i < size; ++i) {
            arr1[i] = arr2[i] * 2.71f;
            arr2[i] = arr1[i] * 0.5f;
        }
    }
}

float verify_sum(float *data, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int test_size = N;
    int use_simd_flag = 1;
    
    /* Parse command line arguments to vary test conditions */
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = N;
    }
    if (argc > 2) {
        use_simd_flag = atoi(argv[2]);
    }
    
    printf("Testing OpenMP SIMT transformation with size=%d, use_simd=%d\n", 
           test_size, use_simd_flag);
    
    /* Allocate and initialize test arrays */
    float *arr1 = (float *)malloc(test_size * sizeof(float));
    float *arr2 = (float *)malloc(test_size * sizeof(float));
    float *arr3 = (float *)malloc(test_size * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < test_size; ++i) {
        arr1[i] = (float)i;
        arr2[i] = (float)(test_size - i);
        arr3[i] = 0.0f;
    }
    
    /* Test 1: Simple vector multiplication with explicit simd clause */
    printf("Test 1: Simple vector multiply\n");
    simple_vector_multiply(arr1, test_size, 2.5f);
    float sum1 = verify_sum(arr1, test_size);
    printf("  Sum after test 1: %f\n", sum1);
    
    /* Test 2: Conditional update without explicit simd clause */
    printf("Test 2: Conditional vector update\n");
    conditional_vector_update(arr2, test_size, THRESHOLD);
    float sum2 = verify_sum(arr2, test_size);
    printf("  Sum after test 2: %f\n", sum2);
    
    /* Test 3: Nested loop computation */
    printf("Test 3: Nested loop computation\n");
    nested_loop_computation(arr1, arr2, arr3, test_size);
    float sum3 = verify_sum(arr3, test_size);
    printf("  Sum after test 3: %f\n", sum3);
    
    /* Test 4: Mixed SIMD/non-SIMD based on runtime condition */
    printf("Test 4: Mixed SIMD mode\n");
    for (int iter = 0; iter < 3; ++iter) {
        mixed_simd_nosimd(arr1, arr2, test_size, (iter % 2));
        float sum4 = verify_sum(arr1, test_size);
        printf("  Iteration %d, sum: %f\n", iter, sum4);
    }
    
    /* Test 5: Multiple target regions with varying parameters */
    printf("Test 5: Parameter sweep\n");
    for (int factor = 1; factor <= 3; ++factor) {
        #pragma omp target teams distribute parallel for \
            map(tofrom: arr3[0:test_size]) num_teams(factor * 2)
        for (int i = 0; i < test_size; ++i) {
            arr3[i] = arr1[i] + arr2[i] * factor;
        }
        float sum5 = verify_sum(arr3, test_size);
        printf("  Factor %d, sum: %f\n", factor, sum5);
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All tests completed\n");
    return 0;
}
