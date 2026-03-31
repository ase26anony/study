/* Test program to cover SIMT transformation in omp-low.cc */
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
        /* Simple vectorizable operation */
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline))
void target_simt_conditional(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Conditional inside loop - creates more complex GIMPLE */
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
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create rich control flow */
        float val = a[i] + b[i];
        if (val < 0) {
            c[i] = -val;
        } else if (val > 100.0f) {
            c[i] = val / 2.0f;
        } else {
            c[i] = val * val;
        }
    }
}

__attribute__((noinline))
void target_mixed_directives(float *arr, int size, int iter) {
    /* Combined directives that might trigger different lowering paths */
    #pragma omp target data map(tofrom: arr[0:size])
    {
        for (int j = 0; j < iter; j++) {
            #pragma omp target teams distribute parallel for simd \
                nowait depend(inout: arr)
            for (int i = 0; i < size; ++i) {
                arr[i] = arr[i] + (float)(i % 16) * 0.1f;
            }
        }
    }
}

float verify_sum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
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
        data3[i] = (float)(i % 100);
        result[i] = 0.0f;
    }
    
    int test_mode = 0;
    if (argc > 1) {
        test_mode = atoi(argv[1]);
    }
    
    /* Execute different target regions based on test mode */
    switch (test_mode) {
        case 0:
            /* Default: run all transformations */
            printf("Running all SIMT transformations...\n");
            target_simt_vector_scale(data1, N, 3.14159f);
            printf("Scale checksum: %f\n", verify_sum(data1, N));
            
            target_simt_conditional(data2, N, THRESHOLD);
            printf("Conditional checksum: %f\n", verify_sum(data2, N));
            
            target_simt_nested_control(data1, data2, result, N);
            printf("Nested control checksum: %f\n", verify_sum(result, N));
            break;
            
        case 1:
            /* Focus on SIMD clause */
            printf("Testing explicit SIMD clause...\n");
            for (int repeat = 0; repeat < 10; ++repeat) {
                target_simt_vector_scale(data3, N, 1.1f);
            }
            printf("Repeated SIMD checksum: %f\n", verify_sum(data3, N));
            break;
            
        case 2:
            /* Test with varying sizes to trigger different optimizations */
            printf("Testing varying sizes...\n");
            for (int size = 128; size <= N; size *= 2) {
                target_simt_conditional(data1, size, (float)size / 2.0f);
                printf("Size %d checksum: %f\n", size, verify_sum(data1, size));
            }
            break;
            
        case 3:
            /* Test mixed directives */
            printf("Testing mixed directives...\n");
            target_mixed_directives(data1, N, 3);
            printf("Mixed directives checksum: %f\n", verify_sum(data1, N));
            break;
            
        default:
            /* Minimal test for coverage */
            printf("Minimal test...\n");
            #pragma omp target teams distribute parallel for \
                map(tofrom: data1[0:64]) simd
            for (int i = 0; i < 64; ++i) {
                data1[i] = data1[i] * 2.0f;
            }
            printf("Minimal checksum: %f\n", verify_sum(data1, 64));
    }
    
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
