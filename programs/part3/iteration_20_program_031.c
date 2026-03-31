/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline))
void target_scale_simd(float *arr, int size, float factor) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * factor + 1.0f;
    }
}

__attribute__((noinline))
void target_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = data[i] * 0.5f;
        } else {
            data[i] = data[i] * 2.0f;
        }
    }
}

__attribute__((noinline))
void target_nested_if_complex(float *a, float *b, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size]) map(from: b[0:size]) num_teams(2)
    for (int i = 0; i < size; ++i) {
        float val = a[i];
        if (val < 0.0f) {
            b[i] = -val;
        } else if (val < 100.0f) {
            b[i] = val * val;
        } else {
            b[i] = val + 100.0f;
        }
    }
}

__attribute__((noinline))
void target_multiple_clauses(int *arr, int size, int offset) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:size]) private(offset) \
        num_teams(16) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] + offset + i;
    }
}

int main(int argc, char *argv[]) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    int *data3 = (int *)malloc(N * sizeof(int));
    
    /* Initialize with test patterns */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(i * 2);
        data3[i] = i % 100;
    }
    
    /* Use command-line arguments to select different kernels */
    int kernel = 0;
    if (argc > 1) {
        kernel = atoi(argv[1]) % 4;
    }
    
    /* Loop over different sizes to increase coverage */
    for (int iter = 0; iter < 3; ++iter) {
        int size = N / (1 << iter);
        if (size < 32) size = 32;
        
        switch (kernel) {
            case 0:
                printf("Running scale_simd kernel, size=%d\n", size);
                target_scale_simd(data1, size, 3.14f);
                break;
            case 1:
                printf("Running conditional_update kernel, size=%d\n", size);
                target_conditional_update(data2, size, THRESHOLD);
                break;
            case 2:
                printf("Running nested_if_complex kernel, size=%d\n", size);
                target_nested_if_complex(data1, data2, size);
                break;
            case 3:
                printf("Running multiple_clauses kernel, size=%d\n", size);
                target_multiple_clauses(data3, size, 10);
                break;
        }
        
        /* Verify computation (simple checksum) */
        double sum = 0.0;
        if (kernel == 0 || kernel == 1 || kernel == 2) {
            for (int i = 0; i < size; ++i) {
                sum += data1[i] + data2[i];
            }
        } else {
            for (int i = 0; i < size; ++i) {
                sum += data3[i];
            }
        }
        printf("Checksum for iteration %d: %f\n", iter, sum);
        
        /* Alternate kernel for next iteration */
        kernel = (kernel + 1) % 4;
    }
    
    /* Additional test with explicit simd clause and collapse */
    #pragma omp target teams distribute parallel for simd collapse(2) \
        map(tofrom: data1[0:N]) num_teams(4)
    for (int i = 0; i < 32; ++i) {
        for (int j = 0; j < 32; ++j) {
            int idx = i * 32 + j;
            if (idx < N) {
                data1[idx] = data1[idx] * 0.25f;
            }
        }
    }
    
    /* Final verification */
    double final_sum = 0.0;
    for (int i = 0; i < N; ++i) {
        final_sum += data1[i] + data2[i] + data3[i];
    }
    printf("Final checksum: %f\n", final_sum);
    
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
