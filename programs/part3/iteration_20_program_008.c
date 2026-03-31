/* Test program to cover SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact */
__attribute__((noinline, target("noinline")))
void kernel_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(16) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale;
    }
}

__attribute__((noinline, target("noinline")))
void kernel_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8) thread_limit(256)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow for GIMPLE sequence */
        if (data[i] > threshold) {
            data[i] = data[i] * 2.0f;
        } else {
            data[i] = data[i] / 2.0f;
        }
    }
}

__attribute__((noinline, target("noinline")))
void kernel_nested_parallelism(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(4) thread_limit(512)
    for (int i = 0; i < size; ++i) {
        /* Vectorizable operation with multiple arrays */
        c[i] = a[i] + b[i] * 3.14f;
        
        /* Additional computation to increase GIMPLE complexity */
        if (i % 2 == 0) {
            c[i] = c[i] - 1.0f;
        }
    }
}

__attribute__((noinline, target("noinline")))
void kernel_multi_clause(float *arr, int size, int offset) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) collapse(1) \
        num_teams(32) thread_limit(64) \
        private(offset) firstprivate(size)
    for (int i = 0; i < size; ++i) {
        /* Access pattern with offset to prevent simple optimizations */
        int idx = (i + offset) % size;
        arr[idx] = arr[idx] * arr[i] + (float)i;
    }
}

float verify_sum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    float *data4 = (float *)malloc(N * sizeof(float));
    
    /* Initialize with test data */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(i * 2);
        data3[i] = (float)(i % 100);
        data4[i] = (float)(i * 3);
    }
    
    /* Use command-line arguments to select kernels */
    int kernel_mask = 0xF; /* Run all kernels by default */
    if (argc > 1) {
        kernel_mask = atoi(argv[1]);
    }
    
    /* Variable loop bounds to trigger different transformations */
    int sizes[] = {256, 512, 1024, 2048};
    int num_sizes = 4;
    
    printf("Starting OpenMP target offloading tests...\n");
    
    /* Execute kernels multiple times with different configurations */
    for (int s = 0; s < num_sizes; s++) {
        int current_size = sizes[s] > N ? N : sizes[s];
        
        if (kernel_mask & 0x1) {
            kernel_vector_scale(data1, current_size, 2.5f);
            printf("Kernel 1 (size=%d): sum = %.2f\n", 
                   current_size, verify_sum(data1, current_size));
        }
        
        if (kernel_mask & 0x2) {
            kernel_conditional_update(data2, current_size, THRESHOLD);
            printf("Kernel 2 (size=%d): sum = %.2f\n", 
                   current_size, verify_sum(data2, current_size));
        }
        
        if (kernel_mask & 0x4) {
            kernel_nested_parallelism(data1, data2, data3, current_size);
            printf("Kernel 3 (size=%d): sum = %.2f\n", 
                   current_size, verify_sum(data3, current_size));
        }
        
        if (kernel_mask & 0x8) {
            kernel_multi_clause(data4, current_size, s * 100);
            printf("Kernel 4 (size=%d): sum = %.2f\n", 
                   current_size, verify_sum(data4, current_size));
        }
    }
    
    /* Final verification */
    float total_sum = verify_sum(data1, N) + verify_sum(data2, N) + 
                     verify_sum(data3, N) + verify_sum(data4, N);
    printf("Total checksum: %.2f\n", total_sum);
    
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    
    return 0;
}
