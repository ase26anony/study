/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_target_teams_distribute_parallel_for_simd(int *arr, int n, int base) {
    volatile int v_bound = n + getpid() % 16; /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(to: v_bound, base) \
                     num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     num_threads(64)
    for (int i = 0; i < v_bound; i++) {
        arr[i] = (i + base) * (i % 32);
    }
    
    /* Compute partial checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (int i = 0; i < n && i < 100; i++) {
        g_checksum += arr[i];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
float test_target_teams_distribute_simd(float *results, int m, int offset) {
    volatile int v_m = m;
    float sum = 0.0f;
    
    #pragma omp target device(ancestor:1) map(tofrom: results[0:m], sum) \
                     map(to: v_m, offset) teams num_teams(8)
    #pragma omp distribute simd dist_schedule(static, 16) reduction(+:sum) \
                     collapse(2)
    for (int i = 0; i < v_m; i += 2) {
        for (int j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx < m) {
                results[idx] = (idx + offset) * 1.5f;
                sum += results[idx];
            }
        }
    }
    
    return sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int rows, int cols, int seed) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    int *device_ptr = NULL;
    size_t size = rows * cols * sizeof(int);
    
    /* Allocate device memory explicitly */
    #pragma omp target data map(to: v_rows, v_cols, seed)
    {
        device_ptr = (int *)omp_target_alloc(size, omp_get_default_device());
        
        if (device_ptr) {
            #pragma omp target is_device_ptr(device_ptr) \
                             map(tofrom: data[0:rows*cols]) if(1)
            #pragma omp teams num_teams(2) thread_limit(256)
            {
                #pragma omp distribute
                for (int i = 0; i < v_rows; i++) {
                    #pragma omp parallel
                    {
                        #pragma omp taskloop simd simdlen(8) nogroup \
                                     num_tasks(omp_get_num_threads() * 4)
                        for (int j = 0; j < v_cols; j++) {
                            int idx = i * cols + j;
                            data[idx] = (i * seed + j * 3) % 256;
                            device_ptr[idx] = data[idx] * 2;
                        }
                    }
                }
            }
            
            /* Copy back from device pointer */
            #pragma omp target is_device_ptr(device_ptr) \
                             map(from: data[0:rows*cols])
            #pragma omp teams distribute parallel for simd
            for (int i = 0; i < rows * cols; i++) {
                data[i] = device_ptr[i];
            }
            
            omp_target_free(device_ptr, omp_get_default_device());
        }
    }
    
    /* Update global counter */
    #pragma omp atomic
    g_volatile_counter++;
}

/* Helper function with runtime bounds */
void test_runtime_bounds(int *arr, int n) {
    int bound = n + (getpid() % 64) - 32;
    bound = bound > 0 ? bound : n;
    
    #pragma omp target if(bound > 512) device(simd:2) \
                     map(tofrom: arr[0:n]) map(to: bound)
    #pragma omp teams distribute parallel for simd \
                     collapse(2) schedule(static, 8)
    for (int i = 0; i < bound; i += 2) {
        for (int j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx < n) {
                arr[idx] = (arr[idx] * 3 + idx) % 1024;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int n = SIZE;
    int m = SIZE / 2;
    int rows = 32, cols = 32;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    float *results = (float *)malloc(m * sizeof(float));
    int *matrix = (int *)malloc(rows * cols * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
    }
    for (int i = 0; i < m; i++) {
        results[i] = 0.0f;
    }
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMD with schedule(simd:static) */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr1, n, 10);
    
    /* Test 2: Distribute simd with reduction */
    printf("Test 2: target teams distribute simd with reduction\n");
    float sum = test_target_teams_distribute_simd(results, m, 20);
    printf("  Reduction sum: %.2f\n", sum);
    
    /* Test 3: Complex nesting with taskloop simd */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(matrix, rows, cols, 42);
    
    /* Test 4: Runtime bounds */
    printf("Test 4: Runtime-dependent bounds\n");
    test_runtime_bounds(arr2, n);
    
    /* Verify results */
    int checksum1 = 0, checksum2 = 0, checksum3 = 0;
    #pragma omp simd reduction(+:checksum1)
    for (int i = 0; i < n && i < 100; i++) checksum1 += arr1[i];
    
    #pragma omp simd reduction(+:checksum2)
    for (int i = 0; i < n && i < 100; i++) checksum2 += arr2[i];
    
    #pragma omp simd reduction(+:checksum3)
    for (int i = 0; i < rows * cols && i < 100; i++) checksum3 += matrix[i];
    
    printf("\nVerification checksums:\n");
    printf("  Array1 checksum (first 100): %d\n", checksum1);
    printf("  Array2 checksum (first 100): %d\n", checksum2);
    printf("  Matrix checksum (first 100): %d\n", checksum3);
    printf("  Global checksum: %d\n", g_checksum);
    printf("  Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(results);
    free(matrix);
    
    printf("\nTests completed successfully!\n");
    return 0;
}
