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
        arr[i] = base + i * 2 + (i % 16);
    }
    
    /* Update global counter */
    #pragma omp atomic
    g_volatile_counter++;
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd_reduction(float *results, int n, int seed) {
    volatile int start = seed;
    volatile int end = n;
    float sum = 0.0f;
    
    /* Allocate device memory explicitly */
    float *d_ptr = (float *)omp_target_alloc(n * sizeof(float), 
                                           omp_get_default_device());
    
    #pragma omp target device(ancestor:1) is_device_ptr(d_ptr) \
                     map(to: start, end) map(tofrom: sum) \
                     dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:sum)
    for (int i = start; i < end; i++) {
        float val = (i % 8 == 0) ? 1.0f : 0.5f;
        d_ptr[i] = val * (i + 1);
        sum += d_ptr[i];
    }
    
    /* Copy back partial results */
    #pragma omp target teams distribute parallel for simd \
                     map(from: results[0:n]) is_device_ptr(d_ptr)
    for (int i = 0; i < n; i++) {
        results[i] = d_ptr[i];
    }
    
    /* Store sum to global checksum */
    #pragma omp atomic
    g_checksum += (int)sum;
    
    omp_target_free(d_ptr, omp_get_default_device());
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *matrix, int rows, int cols) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    #pragma omp target map(tofrom: matrix[0:rows*cols]) \
                     map(to: v_rows, v_cols) if(1)
    {
        #pragma omp teams num_teams(2) thread_limit(32)
        {
            #pragma omp distribute
            for (int i = 0; i < v_rows; i++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(2) grainsize(8) \
                                 num_tasks(16)
                    for (int j = 0; j < v_cols; j++) {
                        for (int k = 0; k < 4; k++) {
                            int idx = i * v_cols + j;
                            matrix[idx] = (i << 16) | (j << 8) | k;
                        }
                    }
                }
            }
        }
    }
    
    /* Nested loop with simd clause */
    #pragma omp target teams distribute parallel for simd \
                 collapse(2) map(tofrom: matrix[0:rows*cols/2])
    for (int i = 0; i < rows/2; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i * cols + j] *= 2;
        }
    }
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

/* Main function with verification */
int main(int argc, char **argv) {
    int n = SIZE;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 64) n = 64;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * n / 2 * sizeof(int));
    float *results = (float *)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = 0;
        results[i] = 0.0f;
    }
    for (int i = 0; i < n * n / 2; i++) {
        arr2[i] = i;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("Using array size: %d\n", n);
    
    /* Test 1: Basic SIMD with schedule */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr1, n, 1000);
    int checksum1 = compute_checksum(arr1, n);
    printf("Checksum 1: %d\n", checksum1);
    
    /* Test 2: Reduction with device pointers */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd_reduction(results, n, n/4);
    
    float sum_results = 0.0f;
    #pragma omp simd reduction(+:sum_results)
    for (int i = 0; i < n; i++) {
        sum_results += results[i];
    }
    printf("Sum of results: %.2f\n", sum_results);
    printf("Global checksum: %d\n", g_checksum);
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(arr2, n/2, n);
    
    int checksum3 = compute_checksum(arr2, n * n / 4);
    printf("Checksum 3: %d\n", checksum3);
    
    /* Verify global counters */
    printf("\nGlobal volatile counter: %d\n", g_volatile_counter);
    
    /* Final verification */
    int final_check = checksum1 + (int)sum_results + checksum3 + g_checksum;
    printf("\nFinal combined checksum: %d\n", final_check);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(results);
    
    return 0;
}
