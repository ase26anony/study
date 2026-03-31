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
    int pid = getpid() % 100;
    volatile int bound = n + pid;
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(from: result[0:BLOCK]) \
                      map(to: bound, n) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) collapse(2)
    for (i = 0; i < bound; i += BLOCK) {
        for (j = 0; j < BLOCK && (i + j) < n; j++) {
            int idx = i + j;
            arr[idx] = idx * 2 + pid;
            if (j < BLOCK) {
                result[j] += arr[idx] % 256;
            }
        }
    }
    
    /* Compute checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < BLOCK; i++) {
        g_checksum += result[i];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int m, float *sum_ptr) {
    int i;
    float local_sum = 0.0f;
    volatile int v_m = m + (getpid() % 50);
    
    #pragma omp target device(ancestor:1) map(to: data[0:v_m]) map(tofrom: local_sum) \
                      map(to: v_m) dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:local_sum)
    for (i = 0; i < v_m; i++) {
        float val = (float)i / (v_m + 1.0f);
        data[i] = val * val;
        local_sum += data[i];
    }
    
    *sum_ptr = local_sum;
    
    /* Update global checksum */
    #pragma omp atomic
    g_checksum += (int)(local_sum * 1000);
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *output) {
    int i, j;
    volatile int v_rows = rows;
    volatile int v_cols = cols + (getpid() % 20);
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(v_rows * v_cols * sizeof(int), 
                                          omp_get_default_device());
    
    if (dev_ptr) {
        #pragma omp target if(1) is_device_ptr(dev_ptr) map(tofrom: matrix[0:v_rows*v_cols]) \
                          map(to: v_rows, v_cols) map(from: output[0:v_rows])
        {
            #pragma omp teams num_teams(4) thread_limit(64)
            {
                #pragma omp distribute
                for (i = 0; i < v_rows; i++) {
                    #pragma omp parallel
                    {
                        #pragma omp taskloop simd simdlen(8) nogroup
                        for (j = 0; j < v_cols; j++) {
                            int idx = i * v_cols + j;
                            dev_ptr[idx] = matrix[idx] * 3;
                            matrix[idx] = dev_ptr[idx] / 2;
                        }
                    }
                    
                    /* Reduction per row */
                    int row_sum = 0;
                    #pragma omp simd reduction(+:row_sum)
                    for (j = 0; j < v_cols; j++) {
                        row_sum += matrix[i * v_cols + j];
                    }
                    output[i] = row_sum;
                }
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    /* Update checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < v_rows; i++) {
        g_checksum += output[i] % 1000;
    }
}

/* Helper function with runtime bounds */
void test_mixed_constructs(int *data, int size) {
    int i;
    volatile int chunk = 16 + (getpid() % 8);
    
    #pragma omp target map(tofrom: data[0:size]) map(to: size, chunk) if(size > 512)
    #pragma omp teams distribute parallel for simd schedule(static, chunk)
    for (i = 0; i < size; i++) {
        data[i] = (data[i] * 7 + i) % 1024;
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float farr[SIZE];
    int matrix[64][64];
    int result[BLOCK] = {0};
    int output[64] = {0};
    float sum = 0.0f;
    
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    
    for (i = 0; i < SIZE; i++) {
        arr[i] = rand() % 1000;
        farr[i] = (float)(rand() % 1000) / 1000.0f;
    }
    
    for (i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d, Seed: %d\n", getpid(), seed);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, SIZE, result);
    
    int checksum1 = 0;
    #pragma omp simd reduction(+:checksum1)
    for (i = 0; i < BLOCK; i++) {
        checksum1 += result[i];
    }
    printf("Checksum 1: %d (global: %d)\n", checksum1, g_checksum);
    
    /* Test 2: SIMD with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(farr, SIZE, &sum);
    printf("Sum: %.3f (global checksum: %d)\n", sum, g_checksum);
    
    /* Test 3: Complex nesting with device pointers */
    printf("\nTest 3: Nested teams with taskloop simd\n");
    test_simt_wrapper_3(&matrix[0][0], 64, 64, output);
    
    int checksum3 = 0;
    #pragma omp simd reduction(+:checksum3)
    for (i = 0; i < 64; i++) {
        checksum3 += output[i];
    }
    printf("Checksum 3: %d (global: %d)\n", checksum3, g_checksum);
    
    /* Test 4: Mixed with runtime condition */
    printf("\nTest 4: Mixed constructs with runtime bounds\n");
    test_mixed_constructs(arr, SIZE);
    
    int final_check = 0;
    #pragma omp simd reduction(+:final_check)
    for (i = 0; i < SIZE; i++) {
        final_check += arr[i];
    }
    printf("Final array checksum: %d\n", final_check);
    printf("Global checksum total: %d\n", g_checksum);
    
    /* Verify results are non-zero */
    if (checksum1 != 0 && sum != 0.0f && checksum3 != 0 && final_check != 0) {
        printf("\nAll tests completed successfully!\n");
        return 0;
    } else {
        printf("\nSome tests produced zero results!\n");
        return 1;
    }
}
