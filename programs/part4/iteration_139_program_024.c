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

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int base) {
    volatile int v_bound = n + getpid() % 16; /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) \
        map(to: v_bound, base) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
        collapse(2) shared(arr)
    for (int i = 0; i < v_bound; i += 2) {
        for (int j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < n) {
                arr[idx] = (i + base) * (j + 1) + (idx % 256);
                g_volatile_counter++; /* Force side effect */
            }
        }
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
int test_simt_wrapper_2(float *results, int m, int seed) {
    int sum = 0;
    volatile int v_m = m + (getpid() & 0xF);
    
    #pragma omp target device(ancestor:1) map(tofrom: results[0:m], sum) \
        map(to: v_m, seed) dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:sum) num_teams(4)
    for (int i = 0; i < v_m; i++) {
        results[i] = (i + seed) * 1.5f;
        sum += (int)results[i];
        g_results[i % SIZE] = i; /* Use global array */
    }
    
    return sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *data, int rows, int cols, int *dev_ptr) {
    volatile int v_rows = rows + (getpid() % 8);
    volatile int v_cols = cols;
    
    #pragma omp target if(1) map(tofrom: data[0:rows*cols]) \
        map(to: v_rows, v_cols) is_device_ptr(dev_ptr)
    #pragma omp teams num_teams(2) thread_limit(64)
    {
        #pragma omp distribute
        for (int i = 0; i < v_rows; i++) {
            #pragma omp parallel
            {
                #pragma omp taskloop simd simdlen(8) \
                    if(0) /* Force conditional evaluation */
                for (int j = 0; j < v_cols; j++) {
                    int idx = i * cols + j;
                    data[idx] = (i << 8) | (j & 0xFF);
                    if (dev_ptr) {
                        data[idx] ^= dev_ptr[idx % 256];
                    }
                    g_volatile_counter += (idx % 3);
                }
            }
        }
    }
}

/* Helper function to allocate device memory */
int* allocate_device_memory(int size) {
    int *ptr = (int*)omp_target_alloc(size * sizeof(int), 
                                      omp_get_default_device());
    if (!ptr) {
        fprintf(stderr, "Device allocation failed\n");
        return NULL;
    }
    
    /* Initialize on device */
    #pragma omp target is_device_ptr(ptr) map(to: size)
    #pragma omp teams distribute parallel for simd
    for (int i = 0; i < size; i++) {
        ptr[i] = i * 3 + 1;
    }
    
    return ptr;
}

int main(int argc, char **argv) {
    int n = SIZE;
    int m = 512;
    int rows = 32, cols = 32;
    
    /* Initialize arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    float *arr2 = (float*)malloc(m * sizeof(float));
    int *arr3 = (int*)malloc(rows * cols * sizeof(int));
    
    for (int i = 0; i < n; i++) arr1[i] = 0;
    for (int i = 0; i < m; i++) arr2[i] = 0.0f;
    for (int i = 0; i < rows * cols; i++) arr3[i] = 0;
    
    /* Allocate device memory for is_device_ptr test */
    int *dev_ptr = allocate_device_memory(256);
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    int base = argc > 1 ? atoi(argv[1]) : 100;
    test_simt_wrapper_1(arr1, n, base);
    
    /* Verify results */
    int checksum1 = 0;
    for (int i = 0; i < n; i++) {
        checksum1 += arr1[i];
    }
    printf("Test 1 checksum: %d\n", checksum1);
    
    /* Test 2: SIMD with reduction */
    int seed = getpid() % 1000;
    int sum = test_simt_wrapper_2(arr2, m, seed);
    
    float checksum2 = 0.0f;
    for (int i = 0; i < m; i++) {
        checksum2 += arr2[i];
    }
    printf("Test 2 - Reduction sum: %d, Array sum: %.2f\n", sum, checksum2);
    
    /* Test 3: Complex nesting with device pointer */
    test_simt_wrapper_3(arr3, rows, cols, dev_ptr);
    
    int checksum3 = 0;
    for (int i = 0; i < rows * cols; i++) {
        checksum3 += arr3[i];
    }
    printf("Test 3 checksum: %d\n", checksum3);
    
    /* Print global volatile counter to prevent dead code elimination */
    printf("Global volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    if (dev_ptr) {
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
