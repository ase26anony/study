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
    
    /* Use volatile to force runtime evaluation */
    volatile int vol_n = n;
    
    /* Complex target region with SIMD schedule */
    #pragma omp target map(to: arr[0:n]) map(from: result[0:n]) \
                      if(0) device(simd:1) num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static, BLOCK) collapse(2) \
                reduction(+:local_sum)
    for (i = 0; i < vol_n; i += 2) {
        for (j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx < n) {
                result[idx] = arr[idx] * 2 + (i % 8);
                local_sum += result[idx];
            }
        }
    }
    
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *output) {
    float sum = 0.0f;
    int i;
    
    /* Runtime-dependent bound */
    int bound = n + (getpid() % 16);
    volatile int vol_bound = bound;
    
    /* Device pointer allocation to force complex data environment */
    float *dev_ptr = (float *)omp_target_alloc(n * sizeof(float), 
                                               omp_get_default_device());
    
    if (dev_ptr) {
        #pragma omp target is_device_ptr(dev_ptr) map(to: data[0:n]) \
                        map(from: output[0:n]) device(ancestor:1)
        #pragma omp teams distribute simd dist_schedule(static, 16) \
                    reduction(+:sum)
        for (i = 0; i < vol_bound; i++) {
            if (i < n) {
                output[i] = data[i] * 3.14f + i;
                sum += output[i];
                dev_ptr[i] = output[i] * 0.5f;
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    g_checksum += (int)sum;
}

/* Function 3: Nested target region with teams and taskloop simd */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    int r, c;
    
    /* Mixed constructs to trigger complex lowering */
    #pragma omp target map(to: matrix[0:rows*cols]) \
                      map(from: row_sums[0:rows]) \
                      if(rows > 0) device(simd:2)
    {
        #pragma omp teams num_teams(rows/16 > 0 ? rows/16 : 1) \
                          thread_limit(64)
        {
            #pragma omp distribute
            for (r = 0; r < rows; r++) {
                int row_sum = 0;
                
                /* Taskloop with simd inside teams distribute */
                #pragma omp taskloop simd reduction(+:row_sum) \
                            grainsize(8) nogroup
                for (c = 0; c < cols; c++) {
                    int idx = r * cols + c;
                    row_sum += matrix[idx] * (r + 1);
                }
                
                row_sums[r] = row_sum;
            }
        }
    }
    
    /* Accumulate to global checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (r = 0; r < rows; r++) {
        g_checksum += row_sums[r];
    }
}

/* Function 4: Additional test with collapse and nowait */
void test_simt_wrapper_4(double *a, double *b, double *c, int n) {
    int i, j;
    
    /* Use argv-dependent bound */
    volatile int vol_n = n + g_volatile_bound;
    
    #pragma omp target map(to: a[0:n*n], b[0:n*n]) map(from: c[0:n*n]) \
                      if(n > 32) device(simd:3)
    #pragma omp teams distribute parallel for simd collapse(2) \
                schedule(static, 8) nowait
    for (i = 0; i < vol_n; i++) {
        for (j = 0; j < n; j++) {
            int idx = i * n + j;
            c[idx] = a[idx] + b[idx] * (i + j);
        }
    }
    
    /* Force synchronization */
    #pragma omp taskwait
}

int main(int argc, char **argv) {
    int i, j;
    
    /* Initialize with runtime-dependent values */
    int base_size = SIZE;
    if (argc > 1) base_size = atoi(argv[1]);
    if (base_size <= 0) base_size = SIZE;
    
    g_volatile_bound = (getpid() % 64);
    
    /* Allocate test arrays */
    int *arr1 = (int *)malloc(base_size * sizeof(int));
    int *res1 = (int *)malloc(base_size * sizeof(int));
    float *arr2 = (float *)malloc(base_size * sizeof(float));
    float *res2 = (float *)malloc(base_size * sizeof(float));
    int matrix_size = (base_size > 256) ? 256 : base_size;
    int *matrix = (int *)malloc(matrix_size * matrix_size * sizeof(int));
    int *row_sums = (int *)malloc(matrix_size * sizeof(int));
    double *a = (double *)malloc(base_size * base_size * sizeof(double));
    double *b = (double *)malloc(base_size * base_size * sizeof(double));
    double *c = (double *)malloc(base_size * base_size * sizeof(double));
    
    if (!arr1 || !res1 || !arr2 || !res2 || !matrix || !row_sums || 
        !a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    #pragma omp parallel for simd
    for (i = 0; i < base_size; i++) {
        arr1[i] = i * 2 + (i % 7);
        arr2[i] = i * 0.5f;
    }
    
    #pragma omp parallel for collapse(2)
    for (i = 0; i < matrix_size; i++) {
        for (j = 0; j < matrix_size; j++) {
            matrix[i * matrix_size + j] = i * matrix_size + j;
        }
    }
    
    #pragma omp parallel for simd collapse(2)
    for (i = 0; i < base_size; i++) {
        for (j = 0; j < base_size; j++) {
            int idx = i * base_size + j;
            a[idx] = i * 1.5;
            b[idx] = j * 2.5;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Execute test functions */
    test_simt_wrapper_1(arr1, base_size, res1);
    printf("Test 1 completed, checksum: %d\n", g_checksum);
    
    test_simt_wrapper_2(arr2, base_size, res2);
    printf("Test 2 completed, checksum: %d\n", g_checksum);
    
    test_simt_wrapper_3(matrix, matrix_size, matrix_size, row_sums);
    printf("Test 3 completed, checksum: %d\n", g_checksum);
    
    test_simt_wrapper_4(a, b, c, base_size);
    printf("Test 4 completed\n");
    
    /* Verify results */
    int final_checksum = 0;
    #pragma omp simd reduction(+:final_checksum)
    for (i = 0; i < base_size; i++) {
        final_checksum += res1[i];
    }
    
    printf("Final checksum: %d (global: %d)\n", final_checksum, g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(res1);
    free(arr2);
    free(res2);
    free(matrix);
    free(row_sums);
    free(a);
    free(b);
    free(c);
    
    return 0;
}
