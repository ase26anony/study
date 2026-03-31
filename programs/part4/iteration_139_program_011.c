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
    int bound = n + (getpid() % 16);
    
    /* Force complex data environment with device pointer */
    int *dev_ptr = (int *)omp_target_alloc(n * sizeof(int), omp_get_default_device());
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) \
                     map(to: bound) map(from: local_sum) is_device_ptr(dev_ptr)
    #pragma omp teams distribute parallel for simd \
                     schedule(simd:static) num_teams(8) thread_limit(64) \
                     reduction(+:local_sum)
    for (i = 0; i < bound; i++) {
        /* Complex computation to prevent dead code elimination */
        int val = i * 2 + (i % 7);
        arr[i] = val;
        dev_ptr[i % n] = val;
        local_sum += val;
    }
    
    *result = local_sum;
    
    /* Copy back from device pointer for verification */
    #pragma omp target is_device_ptr(dev_ptr) map(from: arr[0:n])
    {
        for (i = 0; i < n; i++) {
            arr[i] = dev_ptr[i];
        }
    }
    
    omp_target_free(dev_ptr, omp_get_default_device());
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *sum_result) {
    float sum = 0.0f;
    volatile int v_bound = n + (getpid() % 32);
    
    /* Nested loops with collapse */
    #pragma omp target if(1) device(ancestor:1) map(to: data[0:n]) \
                     map(tofrom: sum) map(to: v_bound)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                     reduction(+:sum) collapse(2)
    for (int i = 0; i < v_bound; i += 2) {
        for (int j = 0; j < BLOCK; j++) {
            float val = (i + j) * 0.5f;
            data[(i * BLOCK + j) % n] = val;
            sum += val;
        }
    }
    
    *sum_result = sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *total) {
    int grand_total = 0;
    int dynamic_bound = rows * cols + (getpid() % 64);
    
    /* Allocate device memory with complex mapping */
    int *dev_matrix = (int *)omp_target_alloc(rows * cols * sizeof(int), 
                                              omp_get_default_device());
    
    #pragma omp target if(0) map(tofrom: matrix[0:rows*cols]) \
                     map(to: dynamic_bound, rows, cols) \
                     map(from: grand_total) is_device_ptr(dev_matrix)
    {
        #pragma omp teams num_teams(4) thread_limit(32)
        {
            #pragma omp distribute
            for (int team = 0; team < 4; team++) {
                int start = team * (rows / 4);
                int end = (team == 3) ? rows : (team + 1) * (rows / 4);
                
                #pragma omp taskloop simd shared(matrix, dev_matrix) \
                             reduction(+:grand_total) collapse(2)
                for (int i = start; i < end; i++) {
                    for (int j = 0; j < cols; j++) {
                        int idx = i * cols + j;
                        int val = (i * 17 + j * 13) % 256;
                        matrix[idx] = val;
                        dev_matrix[idx] = val;
                        grand_total += val;
                    }
                }
            }
        }
    }
    
    *total = grand_total;
    omp_target_free(dev_matrix, omp_get_default_device());
}

/* Helper function to compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int arr1[SIZE];
    float arr2[SIZE];
    int matrix[64][64];
    int result1, total3;
    float result2;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i % 100;
        arr2[i] = i * 0.1f;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = (i + j) % 50;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, SIZE, &result1);
    int checksum1 = compute_checksum(arr1, SIZE);
    printf("Result1 = %d, Checksum1 = %d\n", result1, checksum1);
    g_checksum += checksum1;
    
    /* Test 2: SIMD with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(arr2, SIZE, &result2);
    float checksum2 = 0.0f;
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < SIZE; i++) {
        checksum2 += arr2[i];
    }
    printf("Result2 = %.2f, Checksum2 = %.2f\n", result2, checksum2);
    g_checksum += (int)checksum2;
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Nested teams with taskloop simd\n");
    test_simt_wrapper_3(&matrix[0][0], 64, 64, &total3);
    int checksum3 = 0;
    #pragma omp simd collapse(2) reduction(+:checksum3)
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            checksum3 += matrix[i][j];
        }
    }
    printf("Total3 = %d, Checksum3 = %d\n", total3, checksum3);
    g_checksum += checksum3;
    
    printf("\nFinal global checksum: %d\n", g_checksum);
    
    /* Additional test with runtime arguments */
    if (argc > 1) {
        int extra_size = atoi(argv[1]);
        if (extra_size > 0) {
            int *extra_arr = (int *)malloc(extra_size * sizeof(int));
            int extra_result;
            
            #pragma omp target if(1) device(simd:1) \
                         map(tofrom: extra_arr[0:extra_size]) \
                         map(from: extra_result)
            #pragma omp teams distribute parallel for simd \
                         schedule(static, 32) num_teams(4)
            for (int i = 0; i < extra_size; i++) {
                extra_arr[i] = (i * 3) % 97;
            }
            
            free(extra_arr);
        }
    }
    
    return 0;
}
