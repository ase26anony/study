/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
static int g_results[SIZE] = {0};
static float g_float_results[SIZE] = {0.0f};

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int base) {
    volatile int vol_bound = n + getpid() % 16; /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(to: base, vol_bound) \
                      num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     num_threads(64)
    for (int i = 0; i < vol_bound; i++) {
        arr[i] = base + i * 2;
        g_volatile_counter++; /* Volatile access to prevent dead code elimination */
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, int *sum_ptr) {
    volatile int start = getpid() % 8;
    int local_sum = 0;
    float *dev_ptr = (float*)omp_target_alloc(n * sizeof(float), omp_get_default_device());
    
    if (!dev_ptr) return;
    
    /* Initialize device data */
    #pragma omp target is_device_ptr(dev_ptr) map(to: data[0:n])
    #pragma omp teams distribute simd dist_schedule(static, 16) reduction(+:local_sum) \
                     num_teams(8)
    for (int i = start; i < n - start; i++) {
        dev_ptr[i] = data[i] * 2.0f;
        local_sum += (int)dev_ptr[i];
    }
    
    /* Copy back and accumulate */
    #pragma omp target if(1) device(ancestor:1) map(from: data[0:n]) \
                      is_device_ptr(dev_ptr) map(tofrom: local_sum)
    #pragma omp teams distribute parallel for simd collapse(2) \
                     num_teams(2) thread_limit(32)
    for (int i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i*2 + j;
            if (idx < n) {
                data[idx] = dev_ptr[idx] + (float)local_sum;
                g_float_results[idx] = data[idx]; /* Store to global */
            }
        }
    }
    
    *sum_ptr = local_sum;
    omp_target_free(dev_ptr, omp_get_default_device());
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *out, int rows, int cols) {
    volatile int vol_rows = rows + (getpid() & 3);
    volatile int vol_cols = cols;
    
    #pragma omp target map(tofrom: out[0:rows*cols]) map(to: vol_rows, vol_cols) \
                      device(0) if(vol_rows > 0)
    #pragma omp teams num_teams(vol_rows) thread_limit(64)
    {
        #pragma omp distribute
        for (int i = 0; i < vol_rows; i++) {
            #pragma omp parallel
            {
                #pragma omp taskloop simd simdlen(8) nogroup
                for (int j = 0; j < vol_cols; j++) {
                    int idx = i * vol_cols + j;
                    out[idx] = (i << 16) | (j & 0xFFFF);
                    /* Complex computation to prevent optimization */
                    out[idx] ^= (g_volatile_counter << (idx % 16));
                    g_results[idx % SIZE] = out[idx]; /* Store to global */
                }
            }
        }
    }
}

/* Helper: Initialize array with pattern */
void init_array(int *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (i * 17 + seed) % 100;
    }
}

/* Helper: Initialize float array */
void init_float_array(float *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)((i * 23 + seed) % 100) / 3.0f;
    }
}

/* Helper: Compute checksum */
int compute_checksum(int *arr, int n) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ (i & 0xFF);
    }
    return sum;
}

/* Helper: Compute float checksum */
float compute_float_checksum(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i] * (i % 7 + 1);
    }
    return sum;
}

int main(int argc, char **argv) {
    int n = SIZE;
    int rows = 32, cols = 32;
    
    /* Allocate and initialize test arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    float *arr2 = (float*)malloc(n * sizeof(float));
    int *arr3 = (int*)malloc(rows * cols * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(arr1, n, 42);
    init_float_array(arr2, n, 123);
    init_array(arr3, rows * cols, 789);
    
    printf("Testing SIMT transformation coverage...\n");
    
    /* Test 1: Basic SIMT wrapper with volatile bounds */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, n, 1000);
    int checksum1 = compute_checksum(arr1, n);
    printf("Checksum 1: %d (expected non-zero)\n", checksum1);
    
    /* Test 2: SIMT with reduction and device pointers */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    int sum2 = 0;
    test_simt_wrapper_2(arr2, n, &sum2);
    float checksum2 = compute_float_checksum(arr2, n);
    printf("Sum 2: %d, Checksum 2: %.2f\n", sum2, checksum2);
    
    /* Test 3: Complex nesting with taskloop simd */
    printf("\nTest 3: Nested teams with taskloop simd\n");
    test_simt_wrapper_3(arr3, rows, cols);
    int checksum3 = compute_checksum(arr3, rows * cols);
    printf("Checksum 3: %d\n", checksum3);
    
    /* Verify global arrays were touched */
    int global_sum = 0;
    float global_float_sum = 0.0f;
    #pragma omp parallel for reduction(+:global_sum)
    for (int i = 0; i < SIZE; i++) {
        global_sum += g_results[i];
    }
    
    #pragma omp parallel for reduction(+:global_float_sum)
    for (int i = 0; i < SIZE; i++) {
        global_float_sum += g_float_results[i];
    }
    
    printf("\nGlobal results checksum: %d\n", global_sum);
    printf("Global float results sum: %.2f\n", global_float_sum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("\nAll tests completed.\n");
    
    /* Return non-zero if any checksum is zero (unlikely) */
    return (checksum1 == 0 || checksum3 == 0) ? 1 : 0;
}
