/* Test program to trigger SIMT transformation in OpenMP target offloading */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:size]) \
                num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
                map(tofrom: data[0:size]) \
                num_teams(8) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
                map(to: a[0:size], b[0:size]) \
                map(from: c[0:size]) \
                num_teams(16) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create interesting GIMPLE */
        float val = a[i] + b[i];
        if (val > 100.0f) {
            c[i] = val * 2.0f;
        } else if (val < -100.0f) {
            c[i] = val * 0.1f;
        } else {
            c[i] = sinf(val);
        }
    }
}

__attribute__((noinline))
void target_simt_reduction_like(float *arr, int size, float *sum) {
    float local_sum = 0.0f;
    
    #pragma omp target teams distribute parallel for simd \
                map(to: arr[0:size]) \
                map(tofrom: local_sum) \
                reduction(+:local_sum) \
                num_teams(4)
    for (int i = 0; i < size; ++i) {
        local_sum += arr[i] * arr[i];
    }
    
    *sum = local_sum;
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
    for (int i = 0; i < N; ++i) {
        array1[i] = (float)i;
        array2[i] = (float)(N - i);
        array3[i] = (float)(i % 100);
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < 3; ++iter) {
        int size = N / (iter + 1);
        
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with vector scaling */
                target_simt_vector_scale(array1, size, 3.14159f);
                break;
                
            case 2:
                /* Conditional update without explicit SIMD clause */
                target_simt_conditional_update(array2, size, THRESHOLD);
                break;
                
            case 3:
                /* Complex control flow with multiple arrays */
                target_simt_nested_control(array1, array2, result, size);
                break;
                
            default:
                /* Reduction-like pattern */
                float sum = 0.0f;
                target_simt_reduction_like(array3, size, &sum);
                printf("Iteration %d, sum = %f\n", iter, sum);
                break;
        }
    }
    
    /* Verify computation by computing checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array2[i] + array3[i] + result[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}
