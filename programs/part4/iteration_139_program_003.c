/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_bound = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    int i, j;
    int local_sum = 0;
    
    /* Use runtime-dependent bound to prevent constant folding */
    int bound = n + (getpid() % 16);
    
    /* Force complex data environment with multiple map types */
    #pragma omp target map(tofrom: arr[0:n]) map(from: result[0:BLOCK]) \
                     map(to: bound) if(0) device(simd:1) num_teams(8)
    #pragma omp teams distribute parallel for simd \
                     schedule(simd:static, 32) collapse(2) reduction(+:local_sum)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < n) {
                arr[idx] = i * 1000 + j;
                local_sum += arr[idx];
                result[j] = arr[idx] % 256;
            }
        }
    }
    
    /* Store result to global to prevent dead code elimination */
    #pragma omp atomic
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *partial_sums) {
    int i;
    float sum = 0.0f;
    volatile int vol_n = n;  /* Volatile to prevent optimization */
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(to: data[0:n]) map(from: partial_sums[0:8]) \
                     device(ancestor:1) if(1)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                     reduction(+:sum) num_teams(4) thread_limit(32)
    for (i = 0; i < vol_n; i++) {
        data[i] = (float)i * 1.5f;
        sum += data[i];
        partial_sums[i % 8] += data[i] * 0.5f;
    }
    
    /* Complex control flow to force conditional wrapper */
    if (sum > 0) {
        #pragma omp target update from(data[0:min(n, 64)])
    }
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    int i, j;
    
    /* Allocate device memory explicitly for is_device_ptr */
    size_t matrix_size = rows * cols * sizeof(int);
    int *dev_matrix = (int *)omp_target_alloc(matrix_size, 
                                             omp_get_default_device());
    
    if (dev_matrix == NULL) {
        /* Fallback to regular target if allocation fails */
        #pragma omp target map(tofrom: matrix[0:rows*cols]) \
                         map(from: row_sums[0:rows])
        #pragma omp teams distribute parallel for simd collapse(2)
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                int idx = i * cols + j;
                matrix[idx] = (i + 1) * (j + 1);
                if (j == 0) row_sums[i] = 0;
                row_sums[i] += matrix[idx];
            }
        }
        return;
    }
    
    /* Initialize host data */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            matrix[i * cols + j] = i * cols + j;
        }
    }
    
    /* Use is_device_ptr with explicit device allocation */
    #pragma omp target is_device_ptr(dev_matrix) map(tofrom: row_sums[0:rows]) \
                     if(0) device(simd:2)
    {
        /* Copy data to device */
        #pragma omp parallel for simd collapse(2)
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                dev_matrix[i * cols + j] = matrix[i * cols + j];
            }
        }
        
        /* Teams with taskloop simd - complex nesting */
        #pragma omp teams num_teams(rows/16) thread_limit(64)
        {
            #pragma omp distribute
            for (i = 0; i < rows; i += 16) {
                #pragma omp taskloop simd
                for (j = 0; j < cols; j++) {
                    int start_row = i;
                    int end_row = (i + 16 < rows) ? i + 16 : rows;
                    for (int k = start_row; k < end_row; k++) {
                        row_sums[k] += dev_matrix[k * cols + j];
                    }
                }
            }
        }
    }
    
    /* Copy results back */
    #pragma omp target update from(row_sums[0:rows])
    
    omp_target_free(dev_matrix, omp_get_default_device());
}

/* Helper function with runtime bounds */
void test_variable_bounds(int *output, int max_iter) {
    int i;
    int dynamic_bound = max_iter + (getpid() % 32);
    
    /* Mix of clauses that might trigger SIMT transformation */
    #pragma omp target map(from: output[0:dynamic_bound]) \
                     if(dynamic_bound > 100) device(ancestor:2)
    #pragma omp teams distribute parallel for simd \
                     schedule(static, 8) num_teams(16)
    for (i = 0; i < dynamic_bound; i++) {
        output[i] = i * i;
        /* Access volatile global to prevent optimization */
        output[i] += g_volatile_bound;
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float farr[SIZE];
    int matrix[64][64];
    int row_sums[64];
    int results[BLOCK];
    float partial_sums[8];
    int variable_output[256];
    
    /* Initialize with some pattern */
    for (i = 0; i < SIZE; i++) {
        arr[i] = 0;
        farr[i] = 0.0f;
    }
    
    for (i = 0; i < 64; i++) {
        row_sums[i] = 0;
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = 0;
        }
    }
    
    for (i = 0; i < BLOCK; i++) results[i] = 0;
    for (i = 0; i < 8; i++) partial_sums[i] = 0.0f;
    for (i = 0; i < 256; i++) variable_output[i] = 0;
    
    /* Set volatile bound from command line or pid */
    if (argc > 1) {
        g_volatile_bound = atoi(argv[1]);
    } else {
        g_volatile_bound = getpid() % 100;
    }
    
    printf("Testing SIMT transformation paths...\n");
    printf("Using volatile bound: %d\n", g_volatile_bound);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, SIZE, results);
    
    /* Verify results */
    int sum1 = 0;
    for (i = 0; i < BLOCK; i++) {
        sum1 += results[i];
    }
    printf("Test 1 checksum: %d (global: %d)\n", sum1, g_checksum);
    
    /* Test 2: teams distribute simd with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(farr, SIZE, partial_sums);
    
    float sum2 = 0.0f;
    for (i = 0; i < 8; i++) {
        sum2 += partial_sums[i];
    }
    printf("Test 2 partial sums total: %.2f\n", sum2);
    
    /* Test 3: Complex nesting with device pointers */
    printf("\nTest 3: Nested target with teams and taskloop simd\n");
    test_simt_wrapper_3(&matrix[0][0], 64, 64, row_sums);
    
    int sum3 = 0;
    for (i = 0; i < 64; i++) {
        sum3 += row_sums[i];
    }
    printf("Test 3 matrix row sums total: %d\n", sum3);
    
    /* Test 4: Variable bounds */
    printf("\nTest 4: Variable bounds with runtime condition\n");
    test_variable_bounds(variable_output, 128);
    
    int sum4 = 0;
    for (i = 0; i < 256; i++) {
        sum4 += variable_output[i];
    }
    printf("Test 4 variable output sum: %d\n", sum4);
    
    /* Final verification */
    printf("\nAll tests completed.\n");
    printf("Total global checksum: %d\n", g_checksum);
    
    return 0;
}
