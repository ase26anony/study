/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define N 1024
#define M 512
#define CHUNK_SIZE 16

/* Global variables to prevent optimization */
volatile int global_seed = 0;
static int global_checksum = 0;

/* Device memory pointers */
static int *device_arr = NULL;
static float *device_results = NULL;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int seed) {
    volatile int bound = n;
    int local_seed = seed;
    
    /* Use device clause that might trigger SIMT transformation */
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) \
                     map(to: local_seed, bound) is_device_ptr(device_arr)
    #pragma omp teams distribute parallel for simd \
                     num_teams(4) thread_limit(64) schedule(simd:static, CHUNK_SIZE)
    for (int i = 0; i < bound; i++) {
        arr[i] = (i + local_seed) * 2;
        /* Force SIMD operations */
        arr[i] += (arr[i] % 3) * 7;
    }
}

/* Function 2: target teams distribute simd with reduction */
float test_simt_wrapper_2(float *results, int m, int seed) {
    volatile int rows = m;
    volatile int cols = 16;
    float sum = 0.0f;
    int local_seed = seed;
    
    /* Use ancestor device clause */
    #pragma omp target device(ancestor:1) map(tofrom: results[0:m], sum) \
                     map(to: local_seed, rows, cols)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                     reduction(+:sum) collapse(2)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            float val = (i * cols + j + local_seed) * 1.5f;
            results[i] = val;
            sum += val;
        }
    }
    
    return sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *arr, float *results, int n, int m, int seed) {
    volatile int outer_bound = n / 4;
    volatile int inner_bound = m;
    int local_seed = seed;
    
    /* Mixed constructs that might trigger SIMT transformation */
    #pragma omp target if(1) map(tofrom: arr[0:n], results[0:m]) \
                     map(to: local_seed, outer_bound, inner_bound)
    {
        #pragma omp teams num_teams(2)
        {
            #pragma omp distribute
            for (int team = 0; team < 2; team++) {
                int start = team * (n / 2);
                int end = (team + 1) * (n / 2);
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd simdlen(8) \
                                 if(0) shared(arr, results) firstprivate(local_seed)
                    for (int i = start; i < end; i++) {
                        /* Complex computation to prevent optimization */
                        arr[i] = (i * local_seed) % 97;
                        for (int k = 0; k < 4; k++) {
                            arr[i] += k * (local_seed % 13);
                        }
                        
                        /* Write to results array with SIMD pattern */
                        if (i < m) {
                            results[i] = arr[i] * 0.5f + (i % 8) * 0.125f;
                        }
                    }
                }
            }
        }
    }
}

/* Helper function to allocate device memory */
void allocate_device_memory() {
    device_arr = (int*)omp_target_alloc(N * sizeof(int), omp_get_default_device());
    device_results = (float*)omp_target_alloc(M * sizeof(float), omp_get_default_device());
    
    if (!device_arr || !device_results) {
        fprintf(stderr, "Failed to allocate device memory\n");
        exit(1);
    }
}

/* Helper function to free device memory */
void free_device_memory() {
    if (device_arr) omp_target_free(device_arr, omp_get_default_device());
    if (device_results) omp_target_free(device_results, omp_get_default_device());
}

/* Compute checksum for verification */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum = (sum + arr[i]) % 1000000007;
    }
    return sum;
}

float compute_float_checksum(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char *argv[]) {
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    global_seed = seed;
    
    /* Allocate host arrays */
    int *arr = (int*)malloc(N * sizeof(int));
    float *results = (float*)malloc(M * sizeof(float));
    
    if (!arr || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = i;
    }
    for (int i = 0; i < M; i++) {
        results[i] = 0.0f;
    }
    
    /* Allocate device memory */
    allocate_device_memory();
    
    printf("Starting SIMT transformation tests with seed=%d\n", seed);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, N, seed);
    int checksum1 = compute_checksum(arr, N);
    printf("Checksum 1: %d\n", checksum1);
    global_checksum ^= checksum1;
    
    /* Test 2: SIMD with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    float sum2 = test_simt_wrapper_2(results, M, seed);
    float checksum2 = compute_float_checksum(results, M);
    printf("Sum from reduction: %f\n", sum2);
    printf("Checksum 2: %f\n", checksum2);
    
    /* Reset arrays for test 3 */
    for (int i = 0; i < N; i++) {
        arr[i] = 0;
    }
    for (int i = 0; i < M; i++) {
        results[i] = 0.0f;
    }
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_simt_wrapper_3(arr, results, N, M, seed);
    int checksum3 = compute_checksum(arr, N);
    float checksum3f = compute_float_checksum(results, M);
    printf("Checksum 3 (int): %d\n", checksum3);
    printf("Checksum 3 (float): %f\n", checksum3f);
    
    /* Free memory */
    free(arr);
    free(results);
    free_device_memory();
    
    printf("\nAll tests completed. Global checksum: %d\n", global_checksum);
    
    return 0;
}
