/* test_simt_lowering.c
 * 
 * This program is designed to trigger the SIMT lowering transformation
 * in GCC's omp-low.cc, specifically the uncovered block that generates
 * IFN_GOMP_USE_SIMT and restructures loops for GPU offloading.
 * 
 * Compile with: gcc -O2 -fopenmp -foffload=nvptx-none -fdump-tree-omplower -o test_simt test_simt_lowering.c
 * Run with: ./test_simt [kernel_type] [size]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define DEFAULT_SIZE 1024
#define THRESHOLD 500.0f

/* Prevent inlining to keep target regions intact for lowering */
__attribute__((noinline))
void kernel_vector_scale(float *arr, int N) {
    /* Teams-distribute-parallel-for with simd clause */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:N]) num_teams(4) thread_limit(128)
    for (int i = 0; i < N; ++i) {
        arr[i] = arr[i] * 3.14159f + 1.0f;
    }
}

__attribute__((noinline))
void kernel_conditional_update(float *data, int N, float threshold) {
    /* Teams-distribute-parallel-for with nested if for complex GIMPLE */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:N]) map(to: threshold)
    for (int i = 0; i < N; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]) * 2.0f;
        } else {
            data[i] = data[i] * 0.5f;
        }
    }
}

__attribute__((noinline))
void kernel_multi_array(float *a, float *b, float *c, int N) {
    /* Multiple arrays with independent computations */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:N], b[0:N]) map(from: c[0:N]) \
        num_teams(8) thread_limit(64)
    for (int i = 0; i < N; ++i) {
        float temp = a[i] + b[i];
        c[i] = temp * temp - 2.0f * a[i] * b[i];
    }
}

__attribute__((noinline))
void kernel_nested_control(float *arr, int N) {
    /* More complex control flow within loop body */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:N])
    for (int i = 0; i < N; ++i) {
        float val = arr[i];
        if (val < 0) {
            arr[i] = -val;
        } else if (val > 1000.0f) {
            arr[i] = 1000.0f;
        } else {
            arr[i] = val * val;
        }
    }
}

float compute_checksum(float *data, int N) {
    float sum = 0.0f;
    for (int i = 0; i < N; ++i) {
        sum += data[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int kernel_type = 0;
    int N = DEFAULT_SIZE;
    
    /* Parse command line arguments to vary execution */
    if (argc > 1) kernel_type = atoi(argv[1]);
    if (argc > 2) N = atoi(argv[2]);
    
    if (N <= 0) N = DEFAULT_SIZE;
    printf("Running kernel %d with size %d\n", kernel_type, N);
    
    /* Allocate and initialize test data */
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    
    if (!data1 || !data2 || !data3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with patterned data */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)(i % 100);
        data2[i] = (float)((i * 3) % 100);
        data3[i] = 0.0f;
    }
    
    /* Execute different kernels based on input */
    switch (kernel_type) {
        case 0:
            kernel_vector_scale(data1, N);
            printf("Checksum kernel 0: %f\n", compute_checksum(data1, N));
            break;
            
        case 1:
            kernel_conditional_update(data1, N, THRESHOLD);
            printf("Checksum kernel 1: %f\n", compute_checksum(data1, N));
            break;
            
        case 2:
            kernel_multi_array(data1, data2, data3, N);
            printf("Checksum kernel 2: %f\n", compute_checksum(data3, N));
            break;
            
        case 3:
            kernel_nested_control(data1, N);
            printf("Checksum kernel 3: %f\n", compute_checksum(data1, N));
            break;
            
        default:
            /* Run all kernels sequentially to maximize coverage */
            printf("Running all kernels sequentially:\n");
            
            /* Reset data */
            for (int i = 0; i < N; ++i) {
                data1[i] = (float)(i % 100);
                data2[i] = (float)((i * 3) % 100);
                data3[i] = 0.0f;
            }
            
            kernel_vector_scale(data1, N);
            printf("  Kernel 0 checksum: %f\n", compute_checksum(data1, N));
            
            kernel_conditional_update(data1, N, THRESHOLD);
            printf("  Kernel 1 checksum: %f\n", compute_checksum(data1, N));
            
            /* Reset for kernel 2 */
            for (int i = 0; i < N; ++i) {
                data1[i] = (float)(i % 100);
                data2[i] = (float)((i * 3) % 100);
            }
            
            kernel_multi_array(data1, data2, data3, N);
            printf("  Kernel 2 checksum: %f\n", compute_checksum(data3, N));
            
            kernel_nested_control(data1, N);
            printf("  Kernel 3 checksum: %f\n", compute_checksum(data1, N));
            break;
    }
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    
    return 0;
}
