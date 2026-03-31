#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

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
            data[i] = data[i] * 2.0f - threshold;
        } else {
            data[i] = data[i] * 0.5f + threshold;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Multiple conditional paths to create interesting GIMPLE */
        float val = a[i] + b[i];
        if (val > 1000.0f) {
            c[i] = val * 0.1f;
        } else if (val > 500.0f) {
            c[i] = val * 0.5f;
        } else {
            c[i] = val * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_mixed_directives(float *arr, int size, int iter) {
    /* Combined directives that may trigger different lowering paths */
    #pragma omp target data map(tofrom: arr[0:size])
    {
        for (int k = 0; k < iter; ++k) {
            #pragma omp target teams distribute parallel for simd \
                map(always, tofrom: arr[0:size])
            for (int i = 0; i < size; ++i) {
                arr[i] = arr[i] + (float)k * 0.01f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    /* Initialize test data */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(N - i);
        data3[i] = (float)(i * 2);
    }
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    
    /* Execute different target regions based on input */
    switch (test_case) {
        case 0:
            printf("Running basic SIMT vector scaling\n");
            target_simt_vector_scale(data1, N, 3.14159f);
            break;
            
        case 1:
            printf("Running conditional update with SIMT\n");
            target_simt_conditional_update(data2, N, THRESHOLD);
            break;
            
        case 2:
            printf("Running nested control flow SIMT\n");
            target_simt_nested_control(data1, data2, result, N);
            break;
            
        case 3:
            printf("Running mixed directives with iteration\n");
            target_mixed_directives(data3, N, 5);
            break;
            
        default:
            /* Run all tests to maximize coverage */
            printf("Running all SIMT test cases\n");
            for (int run = 0; run < 3; ++run) {
                target_simt_vector_scale(data1, N, 1.5f);
                target_simt_conditional_update(data2, N, THRESHOLD);
                target_simt_nested_control(data1, data2, result, N/2);
                target_mixed_directives(data3, N/4, 2);
            }
            break;
    }
    
    /* Verify computation wasn't optimized away */
    float checksum = 0.0f;
    #pragma omp parallel for reduction(+:checksum)
    for (int i = 0; i < N; ++i) {
        checksum += data1[i] + data2[i] + data3[i];
        if (i < N && result != NULL) checksum += result[i];
    }
    
    printf("Checksum: %f\n", checksum);
    
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return 0;
}
