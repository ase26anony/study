/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_scale(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor;
    }
}

__attribute__((noinline))
void target_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = data[i] * 2.0f - 1.0f;
        } else {
            data[i] = data[i] * 0.5f + 0.25f;
        }
    }
}

__attribute__((noinline))
void target_nested_if(float *arr, int size) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) collapse(2)
    for (int i = 0; i < size/2; ++i) {
        for (int j = 0; j < 2; ++j) {
            int idx = i*2 + j;
            if (idx % 3 == 0) {
                arr[idx] = arr[idx] * 3.14f;
            } else if (idx % 3 == 1) {
                arr[idx] = arr[idx] + 2.718f;
            } else {
                arr[idx] = arr[idx] - 1.414f;
            }
        }
    }
}

__attribute__((noinline))
void target_simple_add(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size])
    for (int i = 0; i < size; ++i) {
        c[i] = a[i] + b[i];
    }
}

int main(int argc, char **argv) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *result = (float *)malloc(N * sizeof(float));
    
    /* Initialize with test data */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(N - i);
        result[i] = 0.0f;
    }
    
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 4;
    }
    
    /* Execute different target regions based on input */
    for (int iter = 0; iter < 3; ++iter) {
        switch ((test_case + iter) % 4) {
            case 0:
                printf("Running target_scale (iter %d)\n", iter);
                target_scale(data1, N, 1.5f);
                break;
            case 1:
                printf("Running target_conditional_update (iter %d)\n", iter);
                target_conditional_update(data1, N, THRESHOLD);
                break;
            case 2:
                printf("Running target_nested_if (iter %d)\n", iter);
                target_nested_if(data1, N);
                break;
            case 3:
                printf("Running target_simple_add (iter %d)\n", iter);
                target_simple_add(data1, data2, result, N);
                break;
        }
        
        /* Verify computation (prevent dead code elimination) */
        float checksum = 0.0f;
        for (int i = 0; i < N; ++i) {
            checksum += data1[i] + result[i];
        }
        printf("Checksum after iteration %d: %f\n", iter, checksum);
    }
    
    /* Additional test with varying sizes to trigger different paths */
    for (int size = 256; size <= 512; size *= 2) {
        float *small_arr = (float *)malloc(size * sizeof(float));
        for (int i = 0; i < size; ++i) {
            small_arr[i] = (float)i;
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: small_arr[0:size])
        for (int i = 0; i < size; ++i) {
            small_arr[i] = small_arr[i] * 2.0f + (float)(i % 10);
        }
        
        free(small_arr);
    }
    
    free(data1);
    free(data2);
    free(result);
    
    return 0;
}
