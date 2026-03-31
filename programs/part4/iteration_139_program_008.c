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
    
    /* Target region with if clause and device clause - may trigger SIMT wrapper */
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(from: local_sum) \
                       num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                   reduction(+:local_sum)
    for (i = 0; i < bound; i++) {
        arr[i] = i * 2 + 1;
        local_sum += arr[i];
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *sum_result) {
    int i, j;
    float sum = 0.0f;
    volatile int vol_n = n;  /* Volatile to prevent constant propagation */
    
    /* Complex device clause */
    #pragma omp target device(ancestor:1) map(to: data[0:n]) map(tofrom: sum) \
                       num_teams(4)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                   reduction(+:sum) collapse(2)
    for (i = 0; i < vol_n; i += 16) {
        for (j = 0; j < 16 && (i + j) < n; j++) {
            float val = (i + j) * 3.14159f;
            data[i + j] = val;
            sum += val;
        }
    }
    
    *sum_result = sum;
    g_checksum += (int)sum;
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *out, int *in, int n) {
    int i;
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(n * sizeof(int), 
                                          omp_get_default_device());
    if (!dev_ptr) return;
    
    /* Copy data to device */
    #pragma omp target if(1) is_device_ptr(dev_ptr) map(to: in[0:n])
    {
        for (i = 0; i < n; i++) {
            dev_ptr[i] = in[i];
        }
    }
    
    /* Complex nesting that might trigger SIMT transformation */
    #pragma omp target if(0) device(simd:2) map(from: out[0:n]) \
                       is_device_ptr(dev_ptr) num_teams(2)
    {
        #pragma omp teams
        {
            #pragma omp distribute
            for (i = 0; i < n; i += BLOCK) {
                int block_end = (i + BLOCK < n) ? i + BLOCK : n;
                int j;
                
                #pragma omp taskloop simd
                for (j = i; j < block_end; j++) {
                    out[j] = dev_ptr[j] * 3 + 7;
                }
            }
        }
    }
    
    /* Free device memory */
    omp_target_free(dev_ptr, omp_get_default_device());
    
    /* Compute checksum */
    int local_cs = 0;
    for (i = 0; i < n; i++) {
        local_cs += out[i];
    }
    g_checksum += local_cs;
}

/* Additional test with collapse clause */
void test_collapse_simt(double *matrix, int rows, int cols) {
    int i, j;
    double trace = 0.0;
    
    /* Use argv-dependent bounds */
    volatile int vol_rows = rows + (getpid() % 8);
    volatile int vol_cols = cols;
    
    #pragma omp target if(1) device(simd:1) map(tofrom: matrix[0:rows*cols]) \
                       map(tofrom: trace) num_teams(16)
    #pragma omp teams distribute parallel for simd collapse(2) \
                   reduction(+:trace) schedule(static, 8)
    for (i = 0; i < vol_rows; i++) {
        for (j = 0; j < vol_cols; j++) {
            double val = (i == j) ? 1.0 : 0.5;
            matrix[i * cols + j] = val;
            if (i == j) trace += val;
        }
    }
    
    g_checksum += (int)(trace * 1000);
}

int main(int argc, char *argv[]) {
    int i;
    
    /* Initialize with runtime-dependent values */
    int base_size = SIZE;
    if (argc > 1) base_size += atoi(argv[1]) % 100;
    
    /* Allocate test arrays */
    int *arr1 = (int *)malloc(base_size * sizeof(int));
    float *arr2 = (float *)malloc(base_size * sizeof(float));
    int *arr3_in = (int *)malloc(base_size * sizeof(int));
    int *arr3_out = (int *)malloc(base_size * sizeof(int));
    double *matrix = (double *)malloc(base_size * base_size * sizeof(double));
    
    if (!arr1 || !arr2 || !arr3_in || !arr3_out || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (i = 0; i < base_size; i++) {
        arr1[i] = 0;
        arr2[i] = 0.0f;
        arr3_in[i] = i % 100;
        arr3_out[i] = 0;
    }
    for (i = 0; i < base_size * base_size; i++) {
        matrix[i] = 0.0;
    }
    
    /* Set volatile bound */
    g_volatile_bound = base_size / 2 + getpid() % 32;
    
    int result1 = 0;
    float result2 = 0.0f;
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    test_simt_wrapper_1(arr1, base_size, &result1);
    printf("Test 1 completed, result1 = %d\n", result1);
    
    /* Test 2: SIMD with collapse */
    test_simt_wrapper_2(arr2, base_size, &result2);
    printf("Test 2 completed, result2 = %f\n", result2);
    
    /* Test 3: Complex nesting with device pointers */
    test_simt_wrapper_3(arr3_out, arr3_in, base_size);
    printf("Test 3 completed, arr3_out[0] = %d\n", arr3_out[0]);
    
    /* Test 4: Collapse with 2D matrix */
    test_collapse_simt(matrix, base_size / 16, base_size / 16);
    printf("Test 4 completed, matrix[0] = %f\n", matrix[0]);
    
    /* Verify results */
    int final_checksum = 0;
    for (i = 0; i < base_size; i++) {
        final_checksum += arr1[i];
        final_checksum += (int)arr2[i];
        final_checksum += arr3_out[i];
    }
    
    printf("Final checksum: %d (global: %d)\n", final_checksum, g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3_in);
    free(arr3_out);
    free(matrix);
    
    return 0;
}
