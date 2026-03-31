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
    int local_sum = 0;
    int pid_mod = getpid() % 100;
    
    /* Use volatile to prevent constant propagation */
    volatile int vol_n = n + pid_mod;
    
    #pragma omp target map(to: arr[0:n]) map(from: result[0:BLOCK]) \
                      if(pid_mod > 50) device(simd:1) num_teams(8)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static, 32) reduction(+:local_sum)
    for (int i = 0; i < vol_n && i < SIZE; i++) {
        result[i % BLOCK] = arr[i] * 2;
        local_sum += arr[i];
    }
    
    /* Store result to prevent dead code elimination */
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *a, float *b, float *c, int n) {
    float sum = 0.0f;
    int dynamic_bound = n + (getpid() % 64);
    
    /* Complex device clause to trigger SIMT analysis */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                      device(ancestor:1) if(0)
    #pragma omp teams distribute simd \
                dist_schedule(static, 16) reduction(+:sum) collapse(2)
    for (int i = 0; i < dynamic_bound && i < n; i += 2) {
        for (int j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx < n) {
                c[idx] = a[idx] + b[idx] * 2.0f;
                sum += c[idx];
            }
        }
    }
    
    /* Use result */
    volatile float vol_sum = sum;
    g_checksum += (int)vol_sum;
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *data, int rows, int cols, int *output) {
    int pid = getpid();
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(rows * cols * sizeof(int), 
                                          omp_get_default_device());
    if (!dev_ptr) return;
    
    /* Initialize device memory */
    #pragma omp target is_device_ptr(dev_ptr) map(to: data[0:rows*cols])
    #pragma omp teams distribute parallel for simd collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            dev_ptr[i * cols + j] = data[i * cols + j];
        }
    }
    
    /* Complex nesting with taskloop simd */
    #pragma omp target map(from: output[0:rows]) \
                      if(pid % 3 == 0) device(simd:2)
    {
        #pragma omp teams num_teams(4) thread_limit(32)
        {
            #pragma omp distribute
            for (int i = 0; i < rows; i++) {
                int row_sum = 0;
                
                #pragma omp parallel reduction(+:row_sum)
                {
                    #pragma omp taskloop simd
                    for (int j = 0; j < cols; j++) {
                        row_sum += dev_ptr[i * cols + j] * (i + j);
                    }
                }
                
                output[i] = row_sum;
            }
        }
    }
    
    /* Free device memory */
    omp_target_free(dev_ptr, omp_get_default_device());
    
    /* Accumulate results */
    for (int i = 0; i < rows; i++) {
        g_checksum += output[i];
    }
}

/* Function 4: Mixed constructs with runtime bounds */
void test_simt_wrapper_4(double *matrix, int size, double *results) {
    volatile int start = getpid() % 10;
    volatile int end = size - (getpid() % 7);
    
    #pragma omp target map(to: matrix[0:size*size]) map(from: results[0:size]) \
                      if(start > 5) device(ancestor:2)
    #pragma omp teams distribute parallel for simd \
                schedule(static, 8) num_teams(16)
    for (int i = start; i < end && i < size; i++) {
        double row_sum = 0.0;
        
        #pragma omp simd reduction(+:row_sum)
        for (int j = 0; j < size; j++) {
            row_sum += matrix[i * size + j] * (i - j);
        }
        
        results[i] = row_sum;
        
        /* Conditional store to create complex control flow */
        if (row_sum > 100.0) {
            results[i] = row_sum / 2.0;
        }
    }
    
    /* Use results to prevent elimination */
    for (int i = 0; i < size && i < 10; i++) {
        g_checksum += (int)results[i];
    }
}

int main(int argc, char **argv) {
    /* Initialize data with runtime-dependent values */
    int n = SIZE;
    if (argc > 1) n = atoi(argv[1]);
    if (n <= 0) n = SIZE;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *arr2 = (int *)malloc(SIZE * sizeof(int));
    float *fa = (float *)malloc(SIZE * sizeof(float));
    float *fb = (float *)malloc(SIZE * sizeof(float));
    float *fc = (float *)malloc(SIZE * sizeof(float));
    int *matrix_data = (int *)malloc(256 * 256 * sizeof(int));
    int *output = (int *)malloc(256 * sizeof(int));
    double *dmatrix = (double *)malloc(100 * 100 * sizeof(double));
    double *dresults = (double *)malloc(100 * sizeof(double));
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i + 1;
        arr2[i] = (i % BLOCK) * 2;
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)i * 0.5f;
    }
    
    for (int i = 0; i < 256 * 256; i++) {
        matrix_data[i] = (i % 100) + 1;
    }
    
    for (int i = 0; i < 100 * 100; i++) {
        dmatrix[i] = (double)(i % 50) * 0.1;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, n, arr2);
    
    /* Test 2: teams distribute simd with reduction */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(fa, fb, fc, n);
    
    /* Test 3: Complex nesting with device pointers */
    printf("Test 3: Nested target with taskloop simd\n");
    test_simt_wrapper_3(matrix_data, 64, 64, output);
    
    /* Test 4: Mixed constructs with runtime bounds */
    printf("Test 4: Mixed constructs with SIMD clause\n");
    test_simt_wrapper_4(dmatrix, 100, dresults);
    
    /* Verify and print results */
    printf("Final checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(fa);
    free(fb);
    free(fc);
    free(matrix_data);
    free(output);
    free(dmatrix);
    free(dresults);
    
    return 0;
}
