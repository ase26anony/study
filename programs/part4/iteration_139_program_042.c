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
    
    /* Use runtime value to determine bounds */
    int bound = n + getpid() % 16;
    volatile int vol_bound = bound;
    
    /* Complex target region with if clause and device clause */
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n], local_sum) \
                       map(to: vol_bound) is_device_ptr(result) \
                       num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static, 32) collapse(2) reduction(+:local_sum)
    for (i = 0; i < vol_bound; i += 2) {
        for (j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < n) {
                arr[idx] = arr[idx] * 2 + i - j;
                local_sum += arr[idx];
            }
        }
    }
    
    /* Nested loop with different SIMD pattern */
    #pragma omp target device(ancestor:1) map(tofrom: arr[0:n]) \
                       if(n > 512) num_teams(4)
    #pragma omp teams distribute parallel for simd \
                schedule(static, 16) simdlen(8)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] + local_sum % 256;
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *sum) {
    int i, j;
    float local_sum = 0.0f;
    volatile int chunk_size = 16;
    
    /* Device memory allocation for complex data environment */
    float *device_ptr = (float *)omp_target_alloc(n * sizeof(float), 
                                                  omp_get_default_device());
    
    if (device_ptr) {
        #pragma omp target if(1) is_device_ptr(device_ptr) \
                           map(tofrom: data[0:n], local_sum) \
                           map(to: chunk_size) device(simd:2)
        #pragma omp teams distribute simd \
                    dist_schedule(static, chunk_size) reduction(+:local_sum)
        for (i = 0; i < n; i++) {
            data[i] = data[i] * 3.14f + i;
            local_sum += data[i];
            device_ptr[i] = data[i];
        }
        
        /* Second target region with different clause combination */
        #pragma omp target map(from: data[0:n]) is_device_ptr(device_ptr) \
                           if(0) num_teams(2)
        #pragma omp teams distribute parallel for simd \
                    schedule(simd:guided) simdlen(4)
        for (j = 0; j < n/2; j++) {
            int idx = j * 2;
            data[idx] = device_ptr[idx] * device_ptr[idx+1];
        }
        
        omp_target_free(device_ptr, omp_get_default_device());
    }
    
    *sum = local_sum;
    g_checksum += (int)local_sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams region */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *total) {
    int i, j;
    int accum = 0;
    volatile int dyn_bound = rows;
    
    /* Complex target region with multiple nested constructs */
    #pragma omp target map(tofrom: matrix[0:rows*cols], accum) \
                       map(to: dyn_bound, cols) device(ancestor:2) \
                       if(rows > 0 && cols > 0)
    {
        #pragma omp teams num_teams(4) thread_limit(32) \
                          reduction(+:accum)
        {
            #pragma omp distribute
            for (i = 0; i < dyn_bound; i++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(2) \
                                grainsize(8) nogroup
                    for (j = 0; j < cols; j += 2) {
                        for (int k = 0; k < 2; k++) {
                            if (j + k < cols) {
                                int idx = i * cols + j + k;
                                matrix[idx] = matrix[idx] * (i + 1) + (j + k);
                                accum += matrix[idx] % 100;
                            }
                        }
                    }
                }
            }
        }
        
        /* Additional SIMD loop in same target region */
        #pragma omp teams distribute simd \
                    dist_schedule(static, 8) simdlen(16)
        for (i = 0; i < rows * cols; i += 4) {
            for (int k = 0; k < 4 && i + k < rows * cols; k++) {
                matrix[i + k] = matrix[i + k] + accum;
            }
        }
    }
    
    *total = accum;
    g_checksum += accum;
}

/* Helper function with runtime-dependent bounds */
void test_variable_bounds(int *arr, int base_size) {
    int i;
    int size = base_size + (getpid() % 128);
    volatile int vol_size = size;
    
    #pragma omp target if(1) map(tofrom: arr[0:size]) \
                           device(simd:3) num_teams(1)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static) collapse(1)
    for (i = 0; i < vol_size; i++) {
        arr[i] = arr[i] * (i % 16) + (i / 16);
    }
}

int main(int argc, char *argv[]) {
    int i;
    int arr[SIZE];
    float farr[SIZE];
    int matrix[256][256];
    int result1, total3;
    float sum2;
    
    /* Initialize with non-constant values */
    int seed = getpid();
    srand(seed);
    
    for (i = 0; i < SIZE; i++) {
        arr[i] = rand() % 1000;
        farr[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            matrix[i][j] = rand() % 500;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    test_simt_wrapper_1(arr, SIZE, &result1);
    printf("Test 1 result: %d\n", result1);
    
    /* Test 2: SIMD with reduction */
    test_simt_wrapper_2(farr, SIZE, &sum2);
    printf("Test 2 result: %.2f\n", sum2);
    
    /* Test 3: Complex nesting */
    test_simt_wrapper_3(&matrix[0][0], 256, 256, &total3);
    printf("Test 3 result: %d\n", total3);
    
    /* Additional test with variable bounds */
    test_variable_bounds(arr, SIZE/2);
    
    /* Verify results */
    int final_check = 0;
    for (i = 0; i < SIZE; i++) {
        final_check += arr[i] % 256;
        final_check += (int)farr[i] % 256;
    }
    
    for (i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            final_check += matrix[i][j] % 256;
        }
    }
    
    printf("Final checksum: %d (global: %d)\n", final_check, g_checksum);
    
    /* Force compiler to keep all code */
    volatile int dummy = final_check + g_checksum;
    if (dummy > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
