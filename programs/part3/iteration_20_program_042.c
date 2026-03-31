/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered lines 2941-2975.
 * It uses OpenMP target offloading with teams-distribute-parallel-for
 * constructs that are eligible for SIMT transformation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define N 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void kernel_vector_scale(float *arr, int size, float scale) {
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) num_teams(4) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * scale + 1.0f;
    }
}

__attribute__((noinline)) 
void kernel_conditional_update(float *data, int size, float threshold) {
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) num_teams(8)
    for (int i = 0; i < size; ++i) {
        /* Complex enough control flow to create interesting GIMPLE */
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f + threshold;
        }
    }
}

__attribute__((noinline))
void kernel_nested_parallelism(float *a, float *b, float *c, int size) {
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        num_teams(16) thread_limit(64)
    for (int i = 0; i < size; ++i) {
        /* Vectorizable operation with multiple array accesses */
        c[i] = a[i] * b[i] + sinf((float)i * 0.01f);
        
        /* Additional conditional to create more complex GIMPLE */
        if (i % 8 == 0) {
            c[i] = c[i] * 2.0f;
        }
    }
}

__attribute__((noinline))
void kernel_multi_dimensional(float *matrix, int rows, int cols) {
    int total = rows * cols;
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: matrix[0:total]) num_teams(32)
    for (int idx = 0; idx < total; ++idx) {
        int i = idx / cols;
        int j = idx % cols;
        matrix[idx] = (float)(i * j) * 0.1f + matrix[idx];
    }
}

float compute_checksum(float *data, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to select different kernels */
    int kernel_choice = 0;
    int data_size = N;
    
    if (argc > 1) {
        kernel_choice = atoi(argv[1]) % 4;
    }
    if (argc > 2) {
        data_size = atoi(argv[2]);
        if (data_size < 1) data_size = N;
    }
    
    printf("Testing SIMT lowering with kernel %d, size %d\n", 
           kernel_choice, data_size);
    
    /* Allocate and initialize test data */
    float *data1 = (float *)malloc(data_size * sizeof(float));
    float *data2 = (float *)malloc(data_size * sizeof(float));
    float *data3 = (float *)malloc(data_size * sizeof(float));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < data_size; ++i) {
        data1[i] = (float)i * 1.5f;
        data2[i] = (float)(data_size - i) * 0.7f;
        data3[i] = 0.0f;
    }
    
    /* Execute different kernels based on choice to increase coverage */
    switch (kernel_choice) {
        case 0:
            printf("Running vector scaling kernel\n");
            kernel_vector_scale(data1, data_size, 3.14f);
            printf("Checksum data1: %f\n", compute_checksum(data1, data_size));
            break;
            
        case 1:
            printf("Running conditional update kernel\n");
            kernel_conditional_update(data1, data_size, THRESHOLD);
            printf("Checksum data1: %f\n", compute_checksum(data1, data_size));
            break;
            
        case 2:
            printf("Running nested parallelism kernel\n");
            kernel_nested_parallelism(data1, data2, data3, data_size);
            printf("Checksum data3: %f\n", compute_checksum(data3, data_size));
            break;
            
        case 3:
            printf("Running multi-dimensional kernel\n");
            kernel_multi_dimensional(data1, 32, data_size/32);
            printf("Checksum data1: %f\n", compute_checksum(data1, data_size));
            break;
    }
    
    /* Additional iterations to increase chance of hitting the transformation */
    for (int iter = 0; iter < 3; ++iter) {
        int small_size = 256 + iter * 128;
        float *temp = (float *)malloc(small_size * sizeof(float));
        
        for (int i = 0; i < small_size; ++i) {
            temp[i] = (float)i * (iter + 1);
        }
        
        #pragma omp target teams distribute parallel for simd \
            map(tofrom: temp[0:small_size]) num_teams(2)
        for (int i = 0; i < small_size; ++i) {
            temp[i] = temp[i] * 0.25f + (float)iter;
        }
        
        free(temp);
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    printf("Test completed successfully\n");
    return 0;
}
