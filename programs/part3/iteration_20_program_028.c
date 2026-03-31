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
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        collapse(2) num_teams(16)
    for (int i = 0; i < size/2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i * 2 + j;
            /* Multiple conditions to create interesting GIMPLE */
            if (idx % 3 == 0) {
                c[idx] = a[idx] + b[idx];
            } else if (idx % 3 == 1) {
                c[idx] = a[idx] - b[idx];
            } else {
                c[idx] = a[idx] * b[idx];
            }
        }
    }
}

__attribute__((noinline))
void target_simt_reduction(float *arr, int size, float *sum) {
    float local_sum = 0.0f;
    #pragma omp target teams distribute parallel for simd \
        map(to: arr[0:size]) map(from: local_sum) \
        reduction(+:local_sum) num_teams(4)
    for (int i = 0; i < size; ++i) {
        local_sum += arr[i];
    }
    *sum = local_sum;
}

/* Helper function with varying loop bounds */
__attribute__((noinline))
void target_variable_bound(float *arr, int start, int end, float factor) {
    int size = end - start;
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[start:size]) num_teams(2)
    for (int i = start; i < end; ++i) {
        arr[i] = arr[i] * factor + (float)(i - start);
    }
}

int main(int argc, char *argv[]) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    if (!data1 || !data2 || !data3 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(i * 2);
        data3[i] = (float)(i * 3);
        result[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute multiple target regions to increase coverage */
    for (int iter = 0; iter < 3; ++iter) {
        switch (test_case) {
            case 1:
                /* Basic SIMD clause - most likely to trigger SIMT */
                target_simt_vector_scale(data1, N, 2.5f);
                break;
                
            case 2:
                /* Conditional execution within loop */
                target_simt_conditional_update(data2, N, THRESHOLD);
                break;
                
            case 3:
                /* Nested loops with collapse */
                target_simt_nested_control(data1, data2, result, N);
                break;
                
            case 4:
                /* Reduction with SIMD clause */
                {
                    float sum;
                    target_simt_reduction(data3, N, &sum);
                    printf("Reduction sum: %f\n", sum);
                }
                break;
                
            default:
                /* Variable loop bounds */
                target_variable_bound(data1, 0, N/2, 1.5f);
                target_variable_bound(data1, N/2, N, 2.5f);
                break;
        }
        
        /* Mix different target calls */
        if (iter == 1) {
            target_simt_vector_scale(data2, N, 0.5f);
        }
    }
    
    /* Verification computation on host */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += data1[i] + data2[i] + data3[i] + result[i];
    }
    
    printf("Final checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
