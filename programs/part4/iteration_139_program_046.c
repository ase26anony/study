/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_bound = SIZE;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    volatile int local_bound = n;
    int *device_arr = (int *)omp_target_alloc(n * sizeof(int), 0);
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) \
                     is_device_ptr(device_arr) depend(out: arr)
    #pragma omp teams distribute parallel for simd \
                num_teams(4) thread_limit(64) schedule(simd:static, 32)
    for (int i = 0; i < local_bound; i++) {
        arr[i] = i * 2 + (i % 16);
        if (device_arr) device_arr[i] = arr[i] * 3;
    }
    
    /* Process results on host */
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    *result = sum;
    
    if (device_arr) omp_target_free(device_arr, 0);
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *results) {
    volatile int start = getpid() % 100;
    int bound = n + start;
    float sum = 0.0f;
    
    #pragma omp target device(ancestor:1) map(to: data[0:n]) map(from: results[0:n/2]) \
                     map(tofrom: sum) if(bound > 512)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                reduction(+:sum) num_teams(8)
    for (int i = start; i < bound && i < n; i++) {
        float val = data[i] * 2.5f + i;
        results[i % (n/2)] = val;
        sum += val;
    }
    
    /* Nested loop with collapse to trigger complex SIMT handling */
    #pragma omp target teams distribute parallel for simd collapse(2) \
                map(tofrom: results[0:n/2]) if(1)
    for (int i = 0; i < n/16; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < n/2) {
                results[idx] = results[idx] / (sum + 1.0f);
            }
        }
    }
    
    /* Store final result */
    results[0] = sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *output) {
    volatile int row_start = 0;
    int *device_ptr = (int *)omp_target_alloc(rows * cols * sizeof(int), 0);
    
    #pragma omp target if(rows > 32) map(tofrom: matrix[0:rows*cols]) \
                     is_device_ptr(device_ptr) device(simd:2)
    {
        #pragma omp teams num_teams(rows/16) thread_limit(128)
        {
            #pragma omp distribute
            for (int r = row_start; r < rows; r++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd grainsize(8) nogroup
                    for (int c = 0; c < cols; c++) {
                        int idx = r * cols + c;
                        int val = r * c + (r % 8) * (c % 8);
                        matrix[idx] = val;
                        if (device_ptr) device_ptr[idx] = val * 2;
                    }
                }
            }
        }
        
        /* Additional SIMD loop in same target region */
        #pragma omp teams distribute simd
        for (int i = 0; i < rows * cols; i += cols) {
            int row_sum = 0;
            #pragma omp simd reduction(+:row_sum)
            for (int j = 0; j < cols && (i + j) < rows * cols; j++) {
                row_sum += matrix[i + j];
            }
            if (i/cols < rows) {
                output[i/cols] = row_sum;
            }
        }
    }
    
    if (device_ptr) omp_target_free(device_ptr, 0);
}

/* Helper function to verify results */
int verify_results(int *arr, int n, int expected_sum) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return (sum == expected_sum);
}

int main(int argc, char **argv) {
    /* Initialize data with runtime-dependent values */
    int runtime_seed = getpid();
    srand(runtime_seed);
    
    int arr1[SIZE];
    float arr2[SIZE];
    int matrix[64][64];
    int output[64];
    float results[SIZE/2];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = 0;
        }
        output[i] = 0;
    }
    
    for (int i = 0; i < SIZE/2; i++) {
        results[i] = 0.0f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("Runtime seed: %d\n", runtime_seed);
    
    /* Test 1: Basic SIMT wrapper */
    int result1 = 0;
    test_simt_wrapper_1(arr1, SIZE, &result1);
    
    /* Verify Test 1 */
    int expected1 = 0;
    for (int i = 0; i < SIZE; i++) {
        expected1 += i * 2 + (i % 16);
    }
    
    if (verify_results(arr1, SIZE, expected1)) {
        printf("Test 1 PASSED: Result = %d, Expected = %d\n", result1, expected1);
        g_checksum += 1;
    } else {
        printf("Test 1 FAILED: Result = %d, Expected = %d\n", result1, expected1);
    }
    
    /* Test 2: SIMD with reduction */
    test_simt_wrapper_2(arr2, SIZE, results);
    
    /* Verify Test 2 */
    float sum_check = 0.0f;
    int start = getpid() % 100;
    for (int i = start; i < SIZE + start && i < SIZE; i++) {
        sum_check += arr2[i] * 2.5f + i;
    }
    
    float tolerance = 0.001f;
    if (fabs(results[0] - sum_check) < tolerance) {
        printf("Test 2 PASSED: Sum = %f, Expected = %f\n", results[0], sum_check);
        g_checksum += 2;
    } else {
        printf("Test 2 FAILED: Sum = %f, Expected = %f\n", results[0], sum_check);
    }
    
    /* Test 3: Complex nesting */
    test_simt_wrapper_3(&matrix[0][0], 64, 64, output);
    
    /* Verify Test 3 */
    int matrix_sum = 0;
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix_sum += matrix[i][j];
        }
    }
    
    int output_sum = 0;
    for (int i = 0; i < 64; i++) {
        output_sum += output[i];
    }
    
    if (matrix_sum > 0 && output_sum > 0) {
        printf("Test 3 PASSED: Matrix sum = %d, Output sum = %d\n", 
               matrix_sum, output_sum);
        g_checksum += 4;
    } else {
        printf("Test 3 FAILED: Matrix sum = %d, Output sum = %d\n", 
               matrix_sum, output_sum);
    }
    
    /* Final verification */
    printf("\nFinal checksum: %d (expected 7 for all tests passed)\n", g_checksum);
    
    if (g_checksum == 7) {
        printf("All SIMT transformation tests completed successfully!\n");
        return 0;
    } else {
        printf("Some tests failed. Check compilation with -fdump-tree-omplower\n");
        return 1;
    }
}
