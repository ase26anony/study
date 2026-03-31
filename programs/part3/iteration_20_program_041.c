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
void target_region_simple(float *arr, int size) {
    /* Simple vector scaling - highly vectorizable */
    #pragma omp target teams distribute parallel for simd \
        map(tofrom: arr[0:size]) thread_limit(128)
    for (int i = 0; i < size; ++i) {
        arr[i] = arr[i] * 3.14f + 1.0f;
    }
}

__attribute__((noinline))
void target_region_conditional(float *data, int size, float threshold) {
    /* Conditional update inside loop - still vectorizable */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: threshold) \
        num_teams(32) num_threads(64)
    for (int i = 0; i < size; ++i) {
        if (data[i] > threshold) {
            data[i] = sqrtf(data[i]);
        } else {
            data[i] = data[i] * data[i];
        }
    }
}

__attribute__((noinline))
void target_region_complex(float *a, float *b, float *c, int size) {
    /* More complex computation with multiple arrays */
    #pragma omp target teams distribute parallel for \
        map(to: a[0:size], b[0:size]) map(from: c[0:size]) \
        collapse(2) simdlen(8)
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < 4; ++j) {
            int idx = i * 4 + j;
            if (idx < size) {
                c[idx] = a[idx] + b[idx] * 2.0f;
            }
        }
    }
}

__attribute__((noinline))
void target_region_nested_if(float *arr, int size) {
    /* Multiple nested conditionals to create complex GIMPLE */
    #pragma omp target teams distribute parallel for \
        map(tofrom: arr[0:size]) 
    for (int i = 0; i < size; ++i) {
        float val = arr[i];
        if (val > 0.0f) {
            if (val < 100.0f) {
                arr[i] = val * 2.0f;
            } else if (val < 200.0f) {
                arr[i] = val / 2.0f;
            } else {
                arr[i] = sqrtf(val);
            }
        } else {
            arr[i] = fabsf(val);
        }
    }
}

float compute_checksum(float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Use command-line arguments to select different test paths */
    int test_case = 0;
    int iterations = 2;
    
    if (argc > 1) {
        test_case = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    /* Allocate and initialize test data */
    float *data1 = (float *)malloc(N * sizeof(float));
    float *data2 = (float *)malloc(N * sizeof(float));
    float *data3 = (float *)malloc(N * sizeof(float));
    float *data4 = (float *)malloc(N * sizeof(float));
    
    if (!data1 || !data2 || !data3 || !data4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        data1[i] = (float)i;
        data2[i] = (float)(i * 2);
        data3[i] = (float)(i % 100);
        data4[i] = (float)(rand() % 1000);
    }
    
    /* Execute target regions multiple times with different configurations */
    for (int iter = 0; iter < iterations; ++iter) {
        printf("Iteration %d, test_case=%d\n", iter, test_case);
        
        /* Vary loop bounds to trigger different lowering scenarios */
        int current_size = N / (iter + 1);
        if (current_size < 64) current_size = 64;
        
        switch (test_case) {
            case 0:
                /* Test all regions */
                target_region_simple(data1, current_size);
                target_region_conditional(data2, current_size, THRESHOLD);
                target_region_complex(data1, data2, data3, current_size/4);
                target_region_nested_if(data4, current_size);
                break;
                
            case 1:
                /* Focus on SIMD clause */
                target_region_simple(data1, current_size);
                break;
                
            case 2:
                /* Focus on conditional execution */
                target_region_conditional(data2, current_size, THRESHOLD * iter);
                break;
                
            case 3:
                /* Complex nested loops */
                target_region_complex(data1, data2, data3, current_size);
                break;
                
            default:
                /* Mix of all */
                target_region_nested_if(data4, current_size);
                target_region_simple(data1, current_size);
                break;
        }
        
        /* Verify computations to prevent dead code elimination */
        float sum1 = compute_checksum(data1, current_size);
        float sum2 = compute_checksum(data2, current_size);
        float sum3 = compute_checksum(data3, current_size);
        float sum4 = compute_checksum(data4, current_size);
        
        printf("  Checksums: %.2f, %.2f, %.2f, %.2f\n", 
               sum1, sum2, sum3, sum4);
        
        /* Modify data for next iteration */
        for (int i = 0; i < current_size; ++i) {
            data1[i] += 0.1f;
            data2[i] *= 0.9f;
            data3[i] = data4[i];
            data4[i] = (float)(rand() % 1000);
        }
    }
    
    free(data1);
    free(data2);
    free(data3);
    free(data4);
    
    printf("Test completed successfully\n");
    return 0;
}
