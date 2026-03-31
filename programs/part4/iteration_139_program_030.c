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
    int i;
    int local_sum = 0;
    
    /* Use runtime-dependent bound */
    int bound = n + (getpid() % 16);
    
    #pragma omp target map(tofrom: local_sum) map(to: arr[0:n], bound) \
                     if(0) device(simd:1) num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd \
                     schedule(simd:static, 32) reduction(+:local_sum)
    for (i = 0; i < bound; i++) {
        local_sum += arr[i % n] * (i % 8);
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *a, float *b, float *c, int n, float *sum) {
    int i;
    float local_sum = 0.0f;
    volatile int vol_n = n; /* Volatile to prevent constant propagation */
    
    /* Allocate device memory explicitly */
    float *d_a = (float *)omp_target_alloc(n * sizeof(float), 0);
    float *d_b = (float *)omp_target_alloc(n * sizeof(float), 0);
    
    if (d_a && d_b) {
        #pragma omp target data map(to: c[0:n]) \
                               use_device_ptr(d_a, d_b) device(ancestor:1)
        {
            #pragma omp target is_device_ptr(d_a, d_b) map(tofrom: local_sum) \
                         if(1) device(simd:2)
            #pragma omp teams distribute simd dist_schedule(static, 16) \
                         reduction(+:local_sum) collapse(2)
            for (i = 0; i < vol_n; i += 2) {
                for (int j = 0; j < 2; j++) {
                    int idx = i + j;
                    if (idx < n) {
                        d_a[idx] = a[idx] + b[idx];
                        d_b[idx] = a[idx] - b[idx];
                        local_sum += d_a[idx] * d_b[idx] + c[idx];
                    }
                }
            }
        }
        
        /* Copy back results */
        #pragma omp target update from(d_a[0:n])
        #pragma omp target update from(d_b[0:n])
        
        omp_target_free(d_a, 0);
        omp_target_free(d_b, 0);
    }
    
    *sum = local_sum;
    g_checksum += (int)local_sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *data, int rows, int cols, int *total) {
    int i, j;
    int local_total = 0;
    int dynamic_bound = rows * (getpid() % 4 + 1);
    
    #pragma omp target map(to: data[0:rows*cols], cols, dynamic_bound) \
                     map(tofrom: local_total) if(0) device(simd:3)
    {
        #pragma omp teams num_teams(4) thread_limit(128) \
                     reduction(+:local_total)
        {
            #pragma omp distribute
            for (i = 0; i < dynamic_bound; i += rows/4) {
                int end = i + rows/4;
                if (end > dynamic_bound) end = dynamic_bound;
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(2) \
                                 grainsize(8) nogroup
                    for (int ii = i; ii < end; ii++) {
                        for (j = 0; j < cols; j++) {
                            int idx = ii * cols + j;
                            if (idx < rows * cols) {
                                data[idx] = (data[idx] * 3 + 7) % 256;
                                local_total += data[idx];
                            }
                        }
                    }
                }
            }
        }
    }
    
    *total = local_total;
    g_checksum += local_total;
}

/* Helper function with mixed constructs */
void test_mixed_simd(int *out, int n) {
    int i;
    
    #pragma omp target map(tofrom: out[0:n]) if(1) device(simd:4)
    #pragma omp teams distribute parallel for simd \
                 schedule(static, 16) num_teams(16)
    for (i = 0; i < n; i++) {
        out[i] = out[i] * 2 + (i % 3);
    }
    
    /* Nested loop with simd */
    #pragma omp target map(tofrom: out[0:n]) if(0) device(ancestor:2)
    {
        #pragma omp teams distribute simd collapse(2)
        for (i = 0; i < n/2; i++) {
            for (int j = 0; j < 2; j++) {
                int idx = i * 2 + j;
                if (idx < n) {
                    out[idx] = (out[idx] << 1) | (out[idx] >> 31);
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float farr1[SIZE], farr2[SIZE], farr3[SIZE];
    int matrix[64][64];
    int result1, result3;
    float result2;
    
    /* Initialize data with pattern */
    for (i = 0; i < SIZE; i++) {
        arr[i] = i + (getpid() % 100);
        farr1[i] = i * 1.5f;
        farr2[i] = i * 0.75f;
        farr3[i] = i * 2.0f;
    }
    
    for (i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    test_simt_wrapper_1(arr, SIZE, &result1);
    printf("Test 1 result: %d\n", result1);
    
    /* Test 2: With device pointers */
    test_simt_wrapper_2(farr1, farr2, farr3, SIZE, &result2);
    printf("Test 2 result: %f\n", result2);
    
    /* Test 3: Complex nesting */
    test_simt_wrapper_3(&matrix[0][0], 64, 64, &result3);
    printf("Test 3 result: %d\n", result3);
    
    /* Additional mixed test */
    test_mixed_simd(arr, SIZE);
    
    /* Verify results */
    int verify_sum = 0;
    for (i = 0; i < SIZE; i++) {
        verify_sum += arr[i];
    }
    printf("Final checksum: %d (global: %d)\n", verify_sum, g_checksum);
    
    /* Dump some data to prevent dead code elimination */
    if (argc > 1) {
        printf("Sample values: %d %f %d\n", arr[0], farr1[0], matrix[0][0]);
    }
    
    return 0;
}
