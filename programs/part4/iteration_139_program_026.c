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
    int pid = getpid() % 100;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target if(pid > 50) device(simd:1) map(tofrom: arr[0:n]) map(from: result[0:1]) \
                     num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     reduction(+:g_checksum)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + pid;
        g_checksum += arr[i] % 256;
    }
    
    /* Force conditional evaluation */
    volatile int cond = pid;
    if (cond) {
        #pragma omp target if(0) device(ancestor:1) map(tofrom: arr[0:n])
        #pragma omp teams distribute parallel for simd collapse(2) \
                         num_teams(8)
        for (i = 0; i < BLOCK; i++) {
            for (int j = 0; j < BLOCK; j++) {
                int idx = i * BLOCK + j;
                if (idx < n) {
                    arr[idx] += (i + j) * 3;
                }
            }
        }
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *sum) {
    volatile int bound = n;
    float local_sum = 0.0f;
    
    /* Complex device clause combination */
    #pragma omp target if(bound > 512) device(default) map(to: data[0:n]) \
                     map(tofrom: local_sum) is_device_ptr(sum)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                     reduction(+:local_sum)
    for (int i = 0; i < bound; i++) {
        data[i] = data[i] * 1.5f + (float)i;
        local_sum += data[i];
    }
    
    *sum = local_sum;
    
    /* Nested loop with runtime bounds */
    int dynamic_bound = getpid() % 256 + 128;
    #pragma omp target map(tofrom: data[0:dynamic_bound]) if(dynamic_bound > 200)
    #pragma omp teams distribute parallel for simd
    for (int i = 0; i < dynamic_bound; i++) {
        for (int j = 0; j < 4; j++) {
            data[i] += j * 0.25f;
        }
    }
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *output) {
    int i, j;
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(rows * cols * sizeof(int), 
                                          omp_get_default_device());
    
    if (dev_ptr) {
        /* Use is_device_ptr with complex construct */
        #pragma omp target if(rows > 16) device(simd:2) \
                         map(to: matrix[0:rows*cols]) map(from: output[0:rows]) \
                         is_device_ptr(dev_ptr)
        {
            #pragma omp teams num_teams(rows/8) thread_limit(64)
            {
                #pragma omp distribute
                for (i = 0; i < rows; i++) {
                    #pragma omp parallel
                    {
                        #pragma omp taskloop simd grainsize(8) \
                                     reduction(+:output[i])
                        for (j = 0; j < cols; j++) {
                            int idx = i * cols + j;
                            dev_ptr[idx] = matrix[idx] * 3;
                            output[i] += dev_ptr[idx] % 100;
                        }
                    }
                }
            }
        }
        
        /* Copy back and verify */
        #pragma omp target if(1) device(default) is_device_ptr(dev_ptr) \
                         map(from: matrix[0:rows*cols])
        #pragma omp teams distribute parallel for simd collapse(2)
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                int idx = i * cols + j;
                matrix[idx] = dev_ptr[idx];
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
}

/* Helper function with mixed constructs */
void test_mixed_simd(int *a, int *b, int n) {
    volatile int vn = n;
    
    #pragma omp target if(vn) device(ancestor:2) map(tofrom: a[0:n], b[0:n])
    {
        #pragma omp teams distribute parallel for simd \
                     schedule(static, 16) num_teams((n + 255)/256)
        for (int i = 0; i < n; i++) {
            a[i] = i * 2;
        }
        
        #pragma omp teams distribute simd dist_schedule(static)
        for (int i = 0; i < n; i++) {
            b[i] = a[i] + b[i];
        }
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float farr[SIZE];
    int matrix[64][64];
    int output[64];
    float sum = 0.0f;
    
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    
    for (i = 0; i < SIZE; i++) {
        arr[i] = rand() % 1000;
        farr[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (i = 0; i < 64; i++) {
        output[i] = 0;
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, SIZE, &arr[0]);
    
    int checksum1 = 0;
    for (i = 0; i < SIZE; i++) {
        checksum1 += arr[i] % 256;
    }
    printf("Checksum 1: %d (global: %d)\n", checksum1, g_checksum);
    
    /* Test 2: SIMD with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(farr, SIZE, &sum);
    
    float checksum2 = 0.0f;
    for (i = 0; i < SIZE; i++) {
        checksum2 += farr[i];
    }
    printf("Sum: %.2f, Checksum 2: %.2f\n", sum, checksum2);
    
    /* Test 3: Complex nesting with taskloop */
    printf("\nTest 3: Nested teams with taskloop simd\n");
    test_simt_wrapper_3(&matrix[0][0], 64, 64, output);
    
    int checksum3 = 0;
    for (i = 0; i < 64; i++) {
        checksum3 += output[i];
        for (int j = 0; j < 64; j++) {
            checksum3 += matrix[i][j] % 10;
        }
    }
    printf("Checksum 3: %d\n", checksum3);
    
    /* Test 4: Mixed constructs */
    printf("\nTest 4: Mixed SIMD constructs\n");
    int a[256], b[256];
    for (i = 0; i < 256; i++) {
        a[i] = i;
        b[i] = 256 - i;
    }
    test_mixed_simd(a, b, 256);
    
    int checksum4 = 0;
    for (i = 0; i < 256; i++) {
        checksum4 += a[i] + b[i];
    }
    printf("Checksum 4: %d\n", checksum4);
    
    /* Final verification */
    printf("\nAll tests completed.\n");
    printf("Total verification: %d\n", 
           checksum1 + (int)checksum2 + checksum3 + checksum4);
    
    return 0;
}
