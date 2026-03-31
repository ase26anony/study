/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define SIZE 1024
#define BLOCK 64

/* Global variables to prevent optimization */
volatile int g_bound = SIZE;
static int g_checksum = 0;

/* Device memory pointers */
static int *dev_arr = NULL;
static float *dev_results = NULL;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_target_teams_distribute_parallel_for_simd(int *arr, int n) {
    int i;
    volatile int bound = n;
    
    /* Use if(0) to potentially trigger conditional SIMT wrapper */
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:bound]) is_device_ptr(dev_arr)
    #pragma omp teams distribute parallel for simd schedule(simd:static) num_teams(4)
    for (i = 0; i < bound; i++) {
        arr[i] = i * 2 + (int)getpid() % 100;
        if (dev_arr) dev_arr[i] = arr[i]; /* Use device pointer */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (i = 0; i < bound; i++) {
        sum += arr[i];
    }
    #pragma omp atomic
    g_checksum += sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_target_teams_distribute_simd(float *results, int n) {
    int i;
    float sum = 0.0f;
    volatile int chunk = 16;
    
    /* Use device clause to trigger SIMT path */
    #pragma omp target device(ancestor:1) map(tofrom: results[0:n], sum)
    #pragma omp teams distribute simd dist_schedule(static, chunk) reduction(+:sum)
    for (i = 0; i < n; i++) {
        results[i] = (float)i * 3.14f + (float)(getpid() % 1000) * 0.001f;
        sum += results[i];
    }
    
    /* Nested loop with collapse to trigger complex SIMT handling */
    #pragma omp target map(tofrom: results[0:n])
    #pragma omp teams distribute parallel for simd collapse(2) num_teams(2)
    for (i = 0; i < n/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < n) {
                results[idx] += (float)(i + j) * 0.5f;
            }
        }
    }
    
    #pragma omp atomic
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *arr, float *results, int n) {
    int i, j;
    volatile int outer_bound = n / BLOCK;
    
    /* Allocate device memory explicitly */
    if (!dev_arr) {
        dev_arr = (int *)omp_target_alloc(n * sizeof(int), omp_get_default_device());
    }
    if (!dev_results) {
        dev_results = (float *)omp_target_alloc(n * sizeof(float), omp_get_default_device());
    }
    
    /* Complex target region with multiple constructs */
    #pragma omp target if(1) map(tofrom: arr[0:n], results[0:n]) \
                         is_device_ptr(dev_arr, dev_results)
    {
        /* teams distribute with simd */
        #pragma omp teams distribute simd
        for (i = 0; i < n; i++) {
            arr[i] = i * 3;
            if (dev_arr) dev_arr[i] = arr[i];
        }
        
        /* teams with taskloop simd inside */
        #pragma omp teams num_teams(2)
        {
            #pragma omp taskloop simd
            for (i = 0; i < outer_bound; i++) {
                for (j = 0; j < BLOCK; j++) {
                    int idx = i * BLOCK + j;
                    if (idx < n) {
                        results[idx] = (float)arr[idx] * 2.0f;
                        if (dev_results) dev_results[idx] = results[idx];
                    }
                }
            }
        }
        
        /* Another loop with runtime bounds */
        int dynamic_bound = n - (getpid() % 100);
        #pragma omp parallel for simd
        for (i = 0; i < dynamic_bound; i++) {
            arr[i] += (int)results[i];
        }
    }
    
    /* Compute checksum */
    int sum = 0;
    float fsum = 0.0f;
    for (i = 0; i < n; i++) {
        sum += arr[i];
        fsum += results[i];
    }
    #pragma omp atomic
    g_checksum += sum + (int)fsum;
}

/* Helper function with volatile arguments */
void test_volatile_bounds(volatile int bound1, volatile int bound2) {
    int arr[BLOCK * BLOCK];
    
    #pragma omp target map(tofrom: arr[0:bound1*bound2])
    #pragma omp teams distribute parallel for simd collapse(2) schedule(simd:static)
    for (int i = 0; i < bound1; i++) {
        for (int j = 0; j < bound2; j++) {
            arr[i * bound2 + j] = (i + j) * (getpid() % 50);
        }
    }
    
    int sum = 0;
    for (int i = 0; i < bound1 * bound2; i++) {
        sum += arr[i];
    }
    #pragma omp atomic
    g_checksum += sum;
}

int main(int argc, char **argv) {
    int *arr = (int *)malloc(SIZE * sizeof(int));
    float *results = (float *)malloc(SIZE * sizeof(float));
    
    /* Initialize with random-ish values */
    int seed = getpid();
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i + seed) % 100;
        results[i] = (float)((i * seed) % 1000) * 0.001f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d\n", getpid());
    
    /* Test 1: Basic SIMD with schedule */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr, SIZE);
    
    /* Test 2: Teams distribute simd with reduction */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd(results, SIZE);
    
    /* Test 3: Complex nesting */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(arr, results, SIZE);
    
    /* Test 4: Volatile bounds */
    printf("Test 4: Volatile bounds with collapse\n");
    volatile int bound1 = 32;
    volatile int bound2 = 32;
    test_volatile_bounds(bound1, bound2);
    
    /* Verify results */
    printf("\nFinal checksum: %d\n", g_checksum);
    
    /* Print sample values for verification */
    printf("Sample values - arr[0:5]: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\nSample values - results[0:5]: ");
    for (int i = 0; i < 5; i++) {
        printf("%.3f ", results[i]);
    }
    printf("\n");
    
    /* Cleanup device memory */
    if (dev_arr) {
        omp_target_free(dev_arr, omp_get_default_device());
    }
    if (dev_results) {
        omp_target_free(dev_results, omp_get_default_device());
    }
    
    free(arr);
    free(results);
    
    return 0;
}
