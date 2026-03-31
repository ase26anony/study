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
                       num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + pid;
    }
    
    /* Compute checksum */
    result[0] = 0;
    for (i = 0; i < n; i++) {
        result[0] += arr[i];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *a, float *b, int n, float *sum) {
    int i;
    volatile int vol_n = n; /* Volatile to prevent constant propagation */
    
    /* Use ancestor device clause which might trigger SIMT transformation */
    #pragma omp target device(ancestor:1) if(0) \
                       map(to: a[0:n], b[0:n]) map(from: sum[0:1]) \
                       num_teams(4)
    #pragma omp teams distribute simd dist_schedule(static, 16) reduction(+:sum[0])
    for (i = 0; i < vol_n; i++) {
        sum[0] += a[i] * b[i];
    }
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    int i, j;
    int pid = getpid();
    
    /* Allocate device memory explicitly */
    int *d_row_sums = (int *)omp_target_alloc(rows * sizeof(int), 
                                             omp_get_default_device());
    
    if (d_row_sums == NULL) {
        printf("Device allocation failed\n");
        return;
    }
    
    /* Initialize device array */
    #pragma omp target is_device_ptr(d_row_sums) map(always, to: rows)
    #pragma omp teams distribute
    for (i = 0; i < rows; i++) {
        d_row_sums[i] = 0;
    }
    
    /* Main computation with complex nesting */
    #pragma omp target if(pid % 3 == 0) map(tofrom: matrix[0:rows*cols]) \
                       is_device_ptr(d_row_sums) num_teams(2)
    {
        #pragma omp teams distribute collapse(2)
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                #pragma omp taskloop simd grainsize(8) if(0)
                for (int k = 0; k < BLOCK; k++) {
                    /* Simulate some computation */
                    int idx = i * cols + j;
                    matrix[idx] = matrix[idx] + (i + j + k) * pid;
                }
            }
        }
        
        /* Reduction per row */
        #pragma omp teams distribute parallel for simd
        for (i = 0; i < rows; i++) {
            int row_sum = 0;
            #pragma omp simd reduction(+:row_sum)
            for (j = 0; j < cols; j++) {
                row_sum += matrix[i * cols + j];
            }
            d_row_sums[i] = row_sum;
        }
    }
    
    /* Copy results back */
    #pragma omp target is_device_ptr(d_row_sums) map(from: row_sums[0:rows])
    #pragma omp teams distribute parallel for simd
    for (i = 0; i < rows; i++) {
        row_sums[i] = d_row_sums[i];
    }
    
    omp_target_free(d_row_sums, omp_get_default_device());
}

/* Function 4: Mixed constructs with runtime bounds */
void test_simt_wrapper_4(double *data, int *mask, int n, double *output) {
    int i;
    volatile int start = g_volatile_bound;
    
    /* Dynamic bounds based on volatile variable */
    int end = start + n;
    
    /* Use collapse clause which might trigger SIMT transformation */
    #pragma omp target map(to: data[0:n], mask[0:n]) map(from: output[0:n]) \
                       if(start > 0) device(simd:2)
    #pragma omp teams distribute parallel for simd collapse(2) schedule(static, 16)
    for (i = start; i < end; i++) {
        for (int j = 0; j < BLOCK/4; j++) {
            int idx = i * (BLOCK/4) + j;
            if (idx < n && mask[idx]) {
                output[idx] = data[idx] * 3.14159;
            } else {
                output[idx] = 0.0;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int i;
    int test_size = SIZE;
    
    /* Initialize with runtime values */
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = SIZE;
    }
    
    g_volatile_bound = getpid() % 100;
    
    /* Allocate test arrays */
    int *arr1 = (int *)malloc(test_size * sizeof(int));
    float *arr2_a = (float *)malloc(test_size * sizeof(float));
    float *arr2_b = (float *)malloc(test_size * sizeof(float));
    int *matrix = (int *)malloc(test_size * test_size * sizeof(int));
    double *data = (double *)malloc(test_size * sizeof(double));
    int *mask = (int *)malloc(test_size * sizeof(int));
    
    int *row_sums = (int *)malloc(test_size * sizeof(int));
    double *output = (double *)malloc(test_size * sizeof(double));
    
    /* Initialize arrays */
    for (i = 0; i < test_size; i++) {
        arr1[i] = i;
        arr2_a[i] = i * 0.5f;
        arr2_b[i] = i * 0.25f;
        data[i] = i * 1.5;
        mask[i] = (i % 3 == 0) ? 1 : 0;
    }
    
    for (i = 0; i < test_size * test_size; i++) {
        matrix[i] = i % 100;
    }
    
    /* Test 1 */
    int result1[1] = {0};
    test_simt_wrapper_1(arr1, test_size, result1);
    printf("Test 1 checksum: %d\n", result1[0]);
    g_checksum += result1[0];
    
    /* Test 2 */
    float sum2[1] = {0.0f};
    test_simt_wrapper_2(arr2_a, arr2_b, test_size, sum2);
    printf("Test 2 sum: %.2f\n", sum2[0]);
    g_checksum += (int)sum2[0];
    
    /* Test 3 */
    test_simt_wrapper_3(matrix, test_size, test_size, row_sums);
    int row_sum_total = 0;
    for (i = 0; i < test_size; i++) {
        row_sum_total += row_sums[i];
    }
    printf("Test 3 row sums total: %d\n", row_sum_total);
    g_checksum += row_sum_total;
    
    /* Test 4 */
    test_simt_wrapper_4(data, mask, test_size, output);
    double output_sum = 0.0;
    for (i = 0; i < test_size; i++) {
        output_sum += output[i];
    }
    printf("Test 4 output sum: %.2f\n", output_sum);
    g_checksum += (int)output_sum;
    
    printf("Final checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2_a);
    free(arr2_b);
    free(matrix);
    free(data);
    free(mask);
    free(row_sums);
    free(output);
    
    return 0;
}
