#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_simt_vector_multiply(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor + 1.0f;
    }
}

__attribute__((noinline))
void target_simt_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = data[i] * 0.5f;
        } else {
            data[i] = data[i] * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_simt_nested_control(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        /* Nested control flow to create more complex GIMPLE */
        float val = a[i] + b[i];
        if (val > 0.0f) {
            c[i] = val * val;
            if (c[i] > 1000.0f) {
                c[i] = 1000.0f;
            }
        } else {
            c[i] = -val;
        }
    }
}

__attribute__((noinline))
void target_mixed_directives(float *arr, int size) {
    /* Mixed directives to test different lowering paths */
    #pragma omp target data map(tofrom: arr[0:size])
    {
        #pragma omp target teams distribute parallel for simd \
            num_teams(2) thread_limit(128)
        for (int i = 0; i < size; ++i) {
            arr[i] = arr[i] + (float)i;
        }
    }
}

int main(int argc, char *argv[]) {
    float *array1 = (float *)malloc(N * sizeof(float));
    float *array2 = (float *)malloc(N * sizeof(float));
    float *array3 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
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
        switch (test_case) {
            case 1:
                /* Basic SIMD clause with explicit simd */
                target_simt_vector_multiply(array1, N, 3.14f);
                break;
                
            case 2:
                /* Conditional execution within loop */
                target_simt_conditional_update(array2, N, THRESHOLD);
                break;
                
            case 3:
                /* Nested control flow */
                target_simt_nested_control(array1, array2, result, N);
                break;
                
            default:
                /* Mixed directives */
                target_mixed_directives(array3, N);
                break;
        }
        
        /* Vary loop bounds slightly */
        int size = N - iter * 100;
        if (size < 100) size = 100;
        
        /* Call another target function with different size */
        target_simt_vector_multiply(array3, size, 1.5f + (float)iter);
    }
    
    /* Compute verification checksum */
    float checksum = 0.0f;
    for (int i = 0; i < N; ++i) {
        checksum += array1[i] + array2[i] + array3[i] + result[i];
    }
    
    printf("Verification checksum: %f\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(result);
    
    return 0;
}
