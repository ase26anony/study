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
    int pid = getpid();
    volatile int bound = n + (pid % 16); /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(from: result[0:BLOCK]) \
                     map(to: bound) num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     collapse(2) num_threads(32)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < n) {
                result[j] += arr[idx] * (i + 1);
            }
        }
    }
    
    /* Compute partial checksum */
    for (j = 0; j < BLOCK; j++) {
        #pragma omp atomic
        g_checksum += result[j];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *sum) {
    int i;
    volatile int vbound = n;
    float local_sum = 0.0f;
    
    /* Allocate device memory explicitly */
    float *dev_data = (float *)omp_target_alloc(n * sizeof(float), 
                                               omp_get_default_device());
    
    if (dev_data) {
        #pragma omp target device(ancestor:1) is_device_ptr(dev_data) \
                         map(to: vbound) map(tofrom: local_sum) \
                         num_teams(vbound/16) dist_schedule(static, 16)
        #pragma omp teams distribute simd reduction(+:local_sum)
        for (i = 0; i < vbound; i++) {
            dev_data[i] = (float)i * 1.5f;
            local_sum += dev_data[i];
        }
        
        *sum = local_sum;
        omp_target_free(dev_data, omp_get_default_device());
    }
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *output) {
    int i, j;
    volatile int vrows = rows;
    volatile int vcols = cols;
    
    #pragma omp target map(to: matrix[0:rows*cols]) map(from: output[0:rows]) \
                     if(rows > 0) device(simd:2)
    #pragma omp teams num_teams(4) thread_limit(128)
    {
        #pragma omp distribute
        for (i = 0; i < vrows; i++) {
            #pragma omp parallel
            {
                #pragma omp single
                {
                    #pragma omp taskloop simd simdlen(8) nogroup \
                                 private(j) collapse(2)
                    for (i = 0; i < vrows; i++) {
                        for (j = 0; j < vcols; j++) {
                            int idx = i * vcols + j;
                            output[i] += matrix[idx] * (j + 1);
                        }
                    }
                }
            }
        }
    }
}

/* Helper function with runtime bounds */
void test_variable_bounds(int *arr, int size) {
    int i;
    int start = getpid() % 100;
    volatile int end = size - (getpid() % 50);
    
    #pragma omp target if(1) map(tofrom: arr[0:size]) device(simd:3)
    #pragma omp teams distribute parallel for simd \
                     schedule(static, 8) num_teams(16)
    for (i = start; i < end; i++) {
        arr[i] = arr[i] * 2 + i;
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float farr[SIZE];
    int matrix[256][256];
    int result[BLOCK] = {0};
    float sum = 0.0f;
    int output[256] = {0};
    
    /* Initialize with non-zero values */
    for (i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
        farr[i] = (float)(i * 2);
    }
    
    for (i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            matrix[i][j] = i * 256 + j;
        }
    }
    
    /* Set volatile bound from command line if provided */
    if (argc > 1) {
        g_volatile_bound = atoi(argv[1]);
    } else {
        g_volatile_bound = SIZE / 2;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, SIZE, result);
    printf("  Checksum after test 1: %d\n", g_checksum);
    
    /* Test 2: Reduction with device pointers */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(farr, SIZE, &sum);
    printf("  Sum after test 2: %.2f\n", sum);
    
    /* Test 3: Complex nesting */
    printf("Test 3: Nested taskloop simd in teams\n");
    test_simt_wrapper_3(&matrix[0][0], 256, 256, output);
    
    int out_sum = 0;
    for (i = 0; i < 256; i++) {
        out_sum += output[i];
    }
    printf("  Output sum after test 3: %d\n", out_sum);
    
    /* Test 4: Variable bounds */
    printf("Test 4: Variable loop bounds\n");
    test_variable_bounds(arr, SIZE);
    
    /* Verify results */
    int final_check = 0;
    for (i = 0; i < SIZE; i++) {
        final_check += arr[i];
    }
    printf("Final array checksum: %d\n", final_check);
    
    /* Additional test with collapse clause */
    {
        int a[100][100];
        int b[100][100];
        
        #pragma omp target if(0) device(simd:4) map(to: a) map(from: b) \
                         num_teams(4)
        #pragma omp teams distribute parallel for simd collapse(2) \
                         schedule(simd:guided)
        for (i = 0; i < 100; i++) {
            for (int j = 0; j < 100; j++) {
                b[i][j] = a[i][j] * 3;
            }
        }
    }
    
    printf("All tests completed.\n");
    return 0;
}
