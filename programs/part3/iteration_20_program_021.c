/* Test program to trigger SIMT transformation in OpenMP offloading */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_scale(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough body with conditional to create interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) + 1.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(16) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        /* Multiple conditions to create complex GIMPLE sequence */
        float val = a[i] + b[i];
        if (val > 100.0f) {
            c[i] = val * 2.0f;
        } else if (val < -100.0f) {
            c[i] = val * 0.1f;
        } else {
            c[i] = sinf(val) * cosf(val);
        }
    }
}

__attribute__((noinline))
void target_mixed_directives(float *arr, int size, int iter) {
    /* Mix of directives to test different lowering paths */
    #pragma omp target data map(tofrom: arr[0:size])
    {
        for (int j = 0; j < iter; ++j) {
            #pragma omp target teams distribute parallel for simd \
                map(always, tofrom: arr[0:size]) \
                num_teams(2) thread_limit(64)
            for (int i = 0; i < size; ++i) {
                arr[i] = arr[i] + (float)(i % 32) * 0.01f;
            }
        }
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
        data2[i] = (float)(N - i);
        data3[i] = (float)(i * 2);
        result[i] = 0.0f;
    }
    
    /* Use command-line arguments to select different paths */
    int test_case = 1;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute different target regions based on test case */
    switch (test_case) {
        case 1:
            printf("Test case 1: Simple vector scaling with simd clause\n");
            target_simt_vector_scale(data1, N, 3.14159f);
            break;
            
        case 2:
            printf("Test case 2: Conditional update with teams-distribute\n");
            target_simt_conditional_update(data2, N, THRESHOLD);
            break;
            
        case 3:
            printf("Test case 3: Nested control flow with multiple arrays\n");
            target_simt_nested_control(data1, data2, result, N);
            break;
            
        case 4:
            printf("Test case 4: Mixed directives with data region\n");
            target_mixed_directives(data3, N, 3);
            break;
            
        default:
            printf("Test case 0: Execute all transformations\n");
            /* Run all to maximize coverage */
            for (int run = 0; run < 2; ++run) {
                target_simt_vector_scale(data1, N, 1.5f);
                target_simt_conditional_update(data2, N, 250.0f);
                target_simt_nested_control(data1, data2, result, N);
                target_mixed_directives(data3, N, 1);
            }
            break;
    }
    
    /* Verify computation (simple checksum) */
    float checksum = 0.0f;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < N; ++i) {
        checksum += data1[i] + data2[i] + data3[i] + result[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
