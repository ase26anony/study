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
    
    /* Use runtime-dependent bound */
    int bound = n + (getpid() % 16);
    
    /* Force complex data environment with device pointer */
    int *dev_ptr = (int *)omp_target_alloc(n * sizeof(int), omp_get_default_device());
    
    if (dev_ptr) {
        /* Target region with if clause and device clause to trigger SIMT path */
        #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) \
                          map(tofrom: local_sum) is_device_ptr(dev_ptr) \
                          num_teams(4) thread_limit(128)
        #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                         collapse(2) reduction(+:local_sum)
        for (i = 0; i < bound; i += BLOCK) {
            for (j = 0; j < BLOCK && (i + j) < n; j++) {
                int idx = i + j;
                /* Complex computation to prevent dead code elimination */
                arr[idx] = (arr[idx] * 3 + 7) % 256;
                dev_ptr[idx] = arr[idx] ^ 0xFF;
                local_sum += arr[idx] + (dev_ptr[idx] % 16);
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    /* Nested loop with volatile to force code generation */
    volatile int vol_i;
    #pragma omp target if(1) device(ancestor:1) map(tofrom: arr[0:n])
    #pragma omp teams distribute simd dist_schedule(static, 16)
    for (vol_i = 0; vol_i < n/2; vol_i++) {
        arr[vol_i] = arr[vol_i] * 2 - 1;
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with reduction */
void test_simt_wrapper_2(float *data, int n, float *sum_result) {
    float sum = 0.0f;
    int i;
    
    /* Use volatile for runtime variation */
    volatile int start = g_volatile_bound % 8;
    
    /* Multiple map clauses for complex data environment */
    #pragma omp target if(start > 4) device(simd:2) \
                      map(to: data[0:n]) map(from: sum) \
                      num_teams(8)
    #pragma omp teams distribute simd reduction(+:sum) \
                     dist_schedule(static, 32)
    for (i = start; i < n; i++) {
        /* Non-trivial computation */
        float val = data[i];
        data[i] = val * val + 1.0f;
        sum += data[i] / (i + 1);
    }
    
    /* Additional loop with collapse */
    int j;
    #pragma omp target map(tofrom: data[0:n]) if(0)
    #pragma omp teams distribute parallel for simd collapse(2) \
                     schedule(simd:guided)
    for (i = 0; i < n/16; i++) {
        for (j = 0; j < 16; j++) {
            int idx = i * 16 + j;
            if (idx < n) {
                data[idx] = data[idx] * 0.5f + sum;
            }
        }
    }
    
    *sum_result = sum;
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *a, int *b, int *c, int n) {
    int i;
    
    /* Allocate device memory for complex pointer handling */
    int *dev_a = (int *)omp_target_alloc(n * sizeof(int), omp_get_default_device());
    int *dev_b = (int *)omp_target_alloc(n * sizeof(int), omp_get_default_device());
    
    if (dev_a && dev_b) {
        /* Initialize device arrays */
        #pragma omp target if(1) is_device_ptr(dev_a, dev_b) \
                          map(to: a[0:n], b[0:n])
        #pragma omp teams distribute parallel for simd
        for (i = 0; i < n; i++) {
            dev_a[i] = a[i];
            dev_b[i] = b[i];
        }
        
        /* Complex nested construct that may trigger SIMT */
        #pragma omp target if(0) device(simd:3) \
                          map(tofrom: c[0:n]) is_device_ptr(dev_a, dev_b) \
                          num_teams(2)
        {
            #pragma omp teams distribute
            for (i = 0; i < n; i += BLOCK) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd
                    for (int j = i; j < i + BLOCK && j < n; j++) {
                        /* Complex computation with device pointers */
                        c[j] = dev_a[j] * 2 + dev_b[j] * 3;
                        dev_a[j] = c[j] % 100;
                        dev_b[j] = (dev_a[j] + dev_b[j]) / 2;
                    }
                }
            }
        }
        
        /* Copy back results */
        #pragma omp target if(1) is_device_ptr(dev_a) map(from: a[0:n])
        #pragma omp teams distribute simd
        for (i = 0; i < n; i++) {
            a[i] = dev_a[i];
        }
        
        omp_target_free(dev_a, omp_get_default_device());
        omp_target_free(dev_b, omp_get_default_device());
    }
    
    /* Final reduction loop */
    int total = 0;
    #pragma omp target map(tofrom: total) map(to: c[0:n]) if(1)
    #pragma omp teams distribute parallel for simd reduction(+:total) \
                     schedule(simd:dynamic, 8)
    for (i = 0; i < n; i++) {
        total += c[i];
    }
    
    g_checksum += total;
}

/* Helper function to initialize arrays */
void init_array(int *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (i * 3 + seed) % 100;
    }
}

void init_float_array(float *arr, int n, int seed) {
    for (int i = 0; i < n; i++) {
        arr[i] = (float)((i * 7 + seed) % 100) / 10.0f;
    }
}

int main(int argc, char **argv) {
    int n = SIZE;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = SIZE;
    }
    
    /* Initialize data arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *arr3 = (int *)malloc(n * sizeof(int));
    float *farr = (float *)malloc(n * sizeof(float));
    int *results = (int *)malloc(3 * sizeof(int));
    float sum_result = 0.0f;
    
    /* Set volatile bound from runtime */
    g_volatile_bound = getpid() % 100;
    
    /* Initialize arrays with different seeds */
    init_array(arr1, n, 1);
    init_array(arr2, n, 2);
    init_array(arr3, n, 3);
    init_float_array(farr, n, 4);
    
    printf("Starting SIMT transformation tests with n=%d\n", n);
    
    /* Test 1: SIMT wrapper with teams distribute parallel for simd */
    printf("Test 1: teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, n, &results[0]);
    
    /* Test 2: teams distribute simd with reduction */
    printf("Test 2: teams distribute simd with reduction\n");
    test_simt_wrapper_2(farr, n, &sum_result);
    
    /* Test 3: Complex nesting with taskloop simd */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_simt_wrapper_3(arr2, arr3, arr1, n);
    
    /* Verify results */
    int checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < n; i++) {
        checksum1 += arr1[i];
        checksum2 += arr2[i];
        checksum3 += arr3[i];
    }
    
    printf("\nResults:\n");
    printf("Test 1 result: %d, checksum: %d\n", results[0], checksum1);
    printf("Test 2 result: %f\n", sum_result);
    printf("Test 3 checksums: arr2=%d, arr3=%d\n", checksum2, checksum3);
    printf("Global checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    free(results);
    
    return 0;
}
