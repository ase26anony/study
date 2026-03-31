/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define N 1024
#define M 512
#define TEAMS 4
#define THREADS 128

/* Global variables to prevent optimization */
volatile int g_volatile_bound = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    volatile int local_bound = n;
    int sum = 0;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(tofrom: sum) map(to: arr[0:n]) \
                      device(simd:1) if(0) \
                      teams num_teams(TEAMS) thread_limit(THREADS)
    #pragma omp teams distribute parallel for simd \
                      schedule(simd:static, 32) reduction(+:sum)
    for (int i = 0; i < local_bound; i++) {
        sum += arr[i] * (i % 16);
    }
    
    *result = sum;
    g_checksum += sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int m, float *results) {
    volatile int rows = m / 16;
    volatile int cols = 16;
    float total = 0.0f;
    
    /* Use ancestor device clause */
    #pragma omp target map(tofrom: total) map(to: data[0:m]) \
                      device(ancestor:1) if(1) \
                      teams num_teams(rows) dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:total) collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            total += data[idx] * (i + 1) * (j + 1);
        }
    }
    
    /* Nested target region with different construct */
    #pragma omp target map(from: results[0:rows]) map(to: data[0:m]) \
                      if(rows > 0) device(simd:0)
    #pragma omp teams distribute parallel for simd \
                      schedule(static, 8)
    for (int i = 0; i < rows; i++) {
        float row_sum = 0.0f;
        for (int j = 0; j < cols; j++) {
            row_sum += data[i * cols + j];
        }
        results[i] = row_sum;
    }
    
    g_checksum += (int)total;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int size, int *output) {
    volatile int dim = size;
    int *device_ptr = NULL;
    
    /* Allocate device memory explicitly */
    #pragma omp target data map(to: matrix[0:size*size])
    {
        device_ptr = (int *)omp_target_alloc(size * size * sizeof(int), 
                                           omp_get_default_device());
        
        if (device_ptr) {
            /* Copy data to device */
            #pragma omp target is_device_ptr(device_ptr) map(to: matrix[0:size*size])
            {
                for (int i = 0; i < size * size; i++) {
                    device_ptr[i] = matrix[i];
                }
            }
            
            /* Complex target region with multiple constructs */
            #pragma omp target map(from: output[0:size]) \
                              is_device_ptr(device_ptr) \
                              device(simd:1) if(size > 0)
            #pragma omp teams num_teams(size/8) thread_limit(64)
            {
                #pragma omp distribute
                for (int i = 0; i < size; i++) {
                    #pragma omp parallel
                    {
                        #pragma omp taskloop simd grainsize(4) \
                                      if(i % 2 == 0) \
                                      shared(device_ptr, output)
                        for (int j = 0; j < size; j++) {
                            int idx = i * size + j;
                            output[i] += device_ptr[idx] * (i + j);
                        }
                    }
                }
            }
            
            omp_target_free(device_ptr, omp_get_default_device());
        }
    }
    
    /* Calculate checksum */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += output[i];
    }
    g_checksum += sum;
}

/* Helper function with runtime-dependent bounds */
void test_variable_bounds(int argc, char **argv) {
    int bound = 256;
    if (argc > 1) {
        bound = atoi(argv[1]) % 512 + 128;
    }
    
    volatile int dynamic_bound = bound + getpid() % 64;
    int *array = (int *)malloc(dynamic_bound * sizeof(int));
    int result = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < dynamic_bound; i++) {
        array[i] = (i * 3) % 97;
    }
    
    /* Target with SIMD construct and runtime bounds */
    #pragma omp target map(to: array[0:dynamic_bound]) map(from: result) \
                      if(dynamic_bound > 100) device(simd:1)
    #pragma omp teams distribute parallel for simd \
                      num_teams((dynamic_bound + 63)/64) \
                      reduction(+:result)
    for (int i = 0; i < dynamic_bound; i++) {
        result += array[i] * (i % 8);
    }
    
    printf("Variable bounds result: %d\n", result);
    g_checksum += result;
    free(array);
}

int main(int argc, char **argv) {
    /* Initialize data arrays */
    int *arr = (int *)malloc(N * sizeof(int));
    float *fdata = (float *)malloc(M * sizeof(float));
    int *matrix = (int *)malloc(64 * 64 * sizeof(int));
    int *output1 = (int *)malloc(64 * sizeof(int));
    float *results2 = (float *)malloc((M/16) * sizeof(float));
    
    /* Initialize with deterministic but non-trivial patterns */
    for (int i = 0; i < N; i++) {
        arr[i] = (i * 7 + 3) % 113;
    }
    
    for (int i = 0; i < M; i++) {
        fdata[i] = (float)((i * 11) % 89) / 17.0f;
    }
    
    for (int i = 0; i < 64 * 64; i++) {
        matrix[i] = (i % 127) - 63;
    }
    
    for (int i = 0; i < 64; i++) {
        output1[i] = 0;
    }
    
    /* Call test functions */
    int result1 = 0;
    printf("Testing SIMT wrapper 1...\n");
    test_simt_wrapper_1(arr, N, &result1);
    printf("Result 1: %d\n", result1);
    
    printf("Testing SIMT wrapper 2...\n");
    test_simt_wrapper_2(fdata, M, results2);
    printf("Results 2[0]: %.2f\n", results2[0]);
    
    printf("Testing SIMT wrapper 3...\n");
    test_simt_wrapper_3(matrix, 64, output1);
    printf("Output 3[0]: %d\n", output1[0]);
    
    /* Test with variable bounds */
    printf("Testing variable bounds...\n");
    test_variable_bounds(argc, argv);
    
    /* Verify and print final checksum */
    printf("Final checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr);
    free(fdata);
    free(matrix);
    free(output1);
    free(results2);
    
    return 0;
}
