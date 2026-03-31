/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <unistd.h>
#include <string.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_bound = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    int i, j;
    int local_sum = 0;
    
    /* Use runtime-dependent bound */
    int bound = n + (getpid() % 16);
    volatile int vol_bound = bound;
    
    /* Allocate device memory explicitly */
    size_t bytes = n * sizeof(int);
    int *dev_arr = (int *)omp_target_alloc(bytes, 0);
    if (!dev_arr) return;
    
    /* Initialize host array */
    for (i = 0; i < n; i++) {
        arr[i] = i % 100;
    }
    
    /* Copy to device */
    omp_target_memcpy(dev_arr, arr, bytes, 0, 0, 0, 0);
    
    /* Complex target region with if clause and device clause */
    #pragma omp target if(0) device(simd:1) map(tofrom: local_sum) \
                       is_device_ptr(dev_arr) firstprivate(vol_bound)
    #pragma omp teams distribute parallel for simd \
                num_teams(4) thread_limit(64) schedule(simd:static) \
                reduction(+:local_sum) collapse(2)
    for (i = 0; i < vol_bound; i += BLOCK) {
        for (j = 0; j < BLOCK && (i + j) < vol_bound; j++) {
            int idx = i + j;
            dev_arr[idx] = dev_arr[idx] * 2 + 1;
            local_sum += dev_arr[idx];
        }
    }
    
    /* Copy back and accumulate result */
    omp_target_memcpy(arr, dev_arr, bytes, 0, 0, 0, 0);
    *result = local_sum;
    
    /* Free device memory */
    omp_target_free(dev_arr, 0);
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *data, int n, float *sum_result) {
    int i;
    float sum = 0.0f;
    volatile int vol_n = n;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target device(ancestor:1) map(tofrom: sum) \
                       map(to: data[0:n])
    #pragma omp teams distribute simd \
                dist_schedule(static, 16) reduction(+:sum) \
                num_teams(8)
    for (i = 0; i < vol_n; i++) {
        data[i] = data[i] * 3.14159f;
        sum += data[i];
    }
    
    *sum_result = sum;
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *total) {
    int i, j;
    int local_total = 0;
    volatile int vol_rows = rows;
    volatile int vol_cols = cols;
    
    /* Allocate device memory with is_device_ptr */
    size_t matrix_bytes = rows * cols * sizeof(int);
    int *dev_matrix = (int *)omp_target_alloc(matrix_bytes, 0);
    if (!dev_matrix) return;
    
    /* Initialize matrix */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            matrix[i * cols + j] = (i * cols + j) % 50;
        }
    }
    
    omp_target_memcpy(dev_matrix, matrix, matrix_bytes, 0, 0, 0, 0);
    
    /* Complex nesting: target -> teams -> distribute simd */
    #pragma omp target if(1) map(tofrom: local_total) \
                       is_device_ptr(dev_matrix) \
                       firstprivate(vol_rows, vol_cols)
    {
        #pragma omp teams num_teams(2) thread_limit(32)
        {
            #pragma omp distribute simd collapse(2)
            for (i = 0; i < vol_rows; i++) {
                for (j = 0; j < vol_cols; j++) {
                    int idx = i * vol_cols + j;
                    dev_matrix[idx] = dev_matrix[idx] * 3 - 7;
                    local_total += dev_matrix[idx];
                }
            }
            
            /* Additional taskloop simd inside teams region */
            #pragma omp taskloop simd shared(dev_matrix) \
                         reduction(+:local_total) grainsize(8)
            for (i = 0; i < vol_rows * vol_cols; i++) {
                if (dev_matrix[i] > 0) {
                    dev_matrix[i] = dev_matrix[i] / 2;
                    local_total += dev_matrix[i];
                }
            }
        }
    }
    
    omp_target_memcpy(matrix, dev_matrix, matrix_bytes, 0, 0, 0, 0);
    *total = local_total;
    omp_target_free(dev_matrix, 0);
}

/* Helper function to compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int i;
    int test1_result = 0;
    float test2_result = 0.0f;
    int test3_result = 0;
    
    /* Initialize arrays with runtime-dependent sizes */
    int n1 = SIZE + (argc > 1 ? atoi(argv[1]) % 32 : 0);
    int n2 = SIZE / 2;
    int rows = 32, cols = 32;
    
    int *arr1 = (int *)malloc(n1 * sizeof(int));
    float *arr2 = (float *)malloc(n2 * sizeof(float));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    
    if (!arr1 || !arr2 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array 2 with floating point values */
    for (i = 0; i < n2; i++) {
        arr2[i] = (float)(i % 100) * 0.5f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: target teams distribute parallel for simd */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, n1, &test1_result);
    int checksum1 = compute_checksum(arr1, n1);
    printf("  Result: %d, Checksum: %d\n", test1_result, checksum1);
    g_checksum += checksum1;
    
    /* Test 2: target teams distribute simd */
    printf("\nTest 2: target teams distribute simd\n");
    test_simt_wrapper_2(arr2, n2, &test2_result);
    float sum2 = 0.0f;
    for (i = 0; i < n2; i++) sum2 += arr2[i];
    printf("  Result: %.2f, Array sum: %.2f\n", test2_result, sum2);
    g_checksum += (int)sum2;
    
    /* Test 3: Nested target with teams and taskloop simd */
    printf("\nTest 3: Nested target with teams and taskloop simd\n");
    test_simt_wrapper_3(matrix, rows, cols, &test3_result);
    int checksum3 = compute_checksum(matrix, rows * cols);
    printf("  Result: %d, Checksum: %d\n", test3_result, checksum3);
    g_checksum += checksum3;
    
    /* Final verification */
    printf("\nFinal checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    
    return 0;
}
