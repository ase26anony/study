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
void test_target_teams_distribute_parallel_for_simd(int *arr, int n) {
    int i;
    int pid = getpid() % 100;
    
    #pragma omp target map(tofrom: arr[0:n]) if(0) device(simd:1) \
        num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
        schedule(simd:static, BLOCK) private(i)
    for (i = 0; i < n; i++) {
        arr[i] = (i + pid) * 2;
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd_reduction(float *results, int m) {
    int i, j;
    float sum = 0.0f;
    volatile int bound = m + g_volatile_bound;
    
    #pragma omp target map(to: results[0:m]) map(from: sum) \
        device(ancestor:1) if(bound > 0)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
        reduction(+:sum) collapse(2)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < 8; j++) {
            sum += results[i] * (i + j + 1);
        }
    }
    
    /* Store result to prevent dead code elimination */
    results[0] = sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int rows, int cols) {
    int i, j;
    int local_sum = 0;
    
    #pragma omp target map(tofrom: data[0:rows*cols]) \
        if(rows > 0) device(simd:2)
    #pragma omp teams num_teams(2) thread_limit(64)
    {
        #pragma omp distribute
        for (i = 0; i < rows; i++) {
            #pragma omp parallel
            {
                #pragma omp taskloop simd collapse(2) \
                    shared(data) private(j) grainsize(4)
                for (j = 0; j < cols; j++) {
                    int idx = i * cols + j;
                    data[idx] = (data[idx] + i * cols + j) % 256;
                    local_sum += data[idx];
                }
            }
        }
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int *vol_ptr = &data[0];
    *vol_ptr = local_sum;
}

/* Function 4: Using omp_target_alloc with is_device_ptr */
void test_device_ptr(int size) {
    int *device_ptr = NULL;
    int *host_buf = (int *)malloc(size * sizeof(int));
    int i;
    
    /* Initialize host buffer */
    for (i = 0; i < size; i++) {
        host_buf[i] = i * 3;
    }
    
    /* Allocate device memory */
    device_ptr = (int *)omp_target_alloc(size * sizeof(int), 
                                         omp_get_default_device());
    
    if (device_ptr) {
        #pragma omp target is_device_ptr(device_ptr) map(to: host_buf[0:size]) \
            if(size > 0) device(simd:1)
        #pragma omp teams distribute parallel for simd \
            schedule(static, 32)
        for (i = 0; i < size; i++) {
            device_ptr[i] = host_buf[i] * 2 + 1;
        }
        
        /* Copy back and verify */
        #pragma omp target is_device_ptr(device_ptr) map(from: host_buf[0:size])
        #pragma omp teams distribute simd
        for (i = 0; i < size; i++) {
            host_buf[i] = device_ptr[i];
        }
        
        /* Compute checksum */
        for (i = 0; i < size; i++) {
            g_checksum += host_buf[i];
        }
        
        omp_target_free(device_ptr, omp_get_default_device());
    }
    
    free(host_buf);
}

/* Function 5: Mixed constructs with runtime bounds */
void test_mixed_constructs(double *matrix, int n, int m) {
    int i, j;
    volatile int v_n = n;
    volatile int v_m = m;
    
    #pragma omp target map(tofrom: matrix[0:n*m]) \
        if(v_n > 0 && v_m > 0) device(ancestor:2)
    {
        #pragma omp teams distribute parallel for simd \
            schedule(simd:guided) collapse(2)
        for (i = 0; i < v_n; i++) {
            for (j = 0; j < v_m; j++) {
                int idx = i * m + j;
                matrix[idx] = (matrix[idx] + i + j) / (i + j + 1.0);
            }
        }
        
        /* Additional SIMD loop in same region */
        #pragma omp simd
        for (i = 0; i < n; i++) {
            matrix[i * m] *= 2.0;
        }
    }
}

int main(int argc, char **argv) {
    int arr[SIZE];
    float results[SIZE/2];
    int matrix_data[256];
    double dmatrix[16][16];
    int i, j;
    
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    g_volatile_bound = (argc > 1) ? atoi(argv[1]) % 100 : 50;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        arr[i] = rand() % 1000;
    }
    
    for (i = 0; i < SIZE/2; i++) {
        results[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (i = 0; i < 256; i++) {
        matrix_data[i] = i;
    }
    
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            dmatrix[i][j] = i * 16 + j;
        }
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMD transformation */
    test_target_teams_distribute_parallel_for_simd(arr, SIZE);
    
    /* Verify results */
    int sum1 = 0;
    for (i = 0; i < SIZE; i++) {
        sum1 += arr[i];
    }
    printf("Test 1 checksum: %d\n", sum1);
    
    /* Test 2: Reduction with SIMD */
    test_target_teams_distribute_simd_reduction(results, SIZE/2);
    printf("Test 2 first element: %f\n", results[0]);
    
    /* Test 3: Complex nesting */
    test_complex_nesting(matrix_data, 16, 16);
    
    int sum3 = 0;
    for (i = 0; i < 256; i++) {
        sum3 += matrix_data[i];
    }
    printf("Test 3 checksum: %d\n", sum3);
    
    /* Test 4: Device pointer */
    test_device_ptr(128);
    printf("Test 4 global checksum: %d\n", g_checksum);
    
    /* Test 5: Mixed constructs */
    test_mixed_constructs(&dmatrix[0][0], 16, 16);
    
    double sum5 = 0.0;
    for (i = 0; i < 16; i++) {
        for (j = 0; j < 16; j++) {
            sum5 += dmatrix[i][j];
        }
    }
    printf("Test 5 matrix sum: %f\n", sum5);
    
    printf("All tests completed.\n");
    
    return 0;
}
