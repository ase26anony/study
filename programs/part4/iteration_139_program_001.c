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
    volatile int vol_bound = n;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(tofrom: arr[0:n]) if(0) device(simd:1) \
                     num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
                     schedule(simd:static, 16) collapse(2)
    for (i = 0; i < vol_bound; i++) {
        for (int j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < n) {
                arr[idx] = idx * 2 + (i % 4);
            }
        }
    }
    
    /* Compute checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < n; i++) {
        g_checksum += arr[i];
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd(float *results, int n) {
    float sum = 0.0f;
    int base = getpid() % 100; /* Runtime-dependent value */
    
    /* Use ancestor device clause */
    #pragma omp target map(tofrom: results[0:n], sum) device(ancestor:1) \
                     num_teams(8)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                     reduction(+:sum)
    for (int i = 0; i < n; i++) {
        results[i] = (float)(i + base) * 1.5f;
        sum += results[i];
    }
    
    printf("Sum from test 2: %.2f\n", sum);
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int n) {
    int *dev_ptr = NULL;
    int host_data[SIZE];
    
    /* Initialize host data */
    for (int i = 0; i < n; i++) {
        host_data[i] = i * 3;
    }
    
    /* Allocate device memory */
    dev_ptr = (int *)omp_target_alloc(n * sizeof(int), 0);
    if (!dev_ptr) {
        printf("Device allocation failed\n");
        return;
    }
    
    /* Copy data to device */
    #pragma omp target is_device_ptr(dev_ptr) map(to: host_data[0:n])
    {
        #pragma omp teams num_teams(2)
        {
            #pragma omp distribute
            for (int team = 0; team < 2; team++) {
                /* Nested SIMD loop inside teams */
                #pragma omp parallel for simd
                for (int i = team * (n/2); i < (team + 1) * (n/2); i++) {
                    dev_ptr[i] = host_data[i] + team;
                }
                
                /* Taskloop with SIMD inside teams region */
                #pragma omp taskloop simd grainsize(32)
                for (int i = team * (n/2); i < (team + 1) * (n/2); i++) {
                    dev_ptr[i] += i % 8;
                }
            }
        }
    }
    
    /* Copy back and verify */
    #pragma omp target is_device_ptr(dev_ptr) map(from: data[0:n])
    {
        #pragma omp simd
        for (int i = 0; i < n; i++) {
            data[i] = dev_ptr[i];
        }
    }
    
    omp_target_free(dev_ptr, 0);
}

/* Additional test with runtime bounds and collapse */
void test_runtime_collapse(int *arr, int m, int n) {
    volatile int rows = m;
    volatile int cols = n;
    
    /* Mixed device clauses */
    #pragma omp target map(tofrom: arr[0:m*n]) if(1) device(simd:0)
    #pragma omp teams distribute parallel for simd collapse(2) \
                     num_teams(4) thread_limit(64)
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            arr[idx] = (i * 17 + j * 13) % 256;
        }
    }
}

int main(int argc, char **argv) {
    int arr1[SIZE * BLOCK];
    float arr2[SIZE];
    int arr3[SIZE];
    int arr4[256 * 256];
    int n = SIZE;
    
    /* Initialize with runtime values */
    int seed = getpid();
    srand(seed);
    g_volatile_bound = 100 + (seed % 50);
    
    printf("Starting SIMT transformation tests (PID: %d)\n", seed);
    
    /* Test 1: Basic SIMT transformation */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr1, n);
    printf("Checksum 1: %d\n", g_checksum);
    
    /* Test 2: Reduction with SIMD */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd(arr2, n);
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(arr3, n);
    
    /* Verify results from test 3 */
    int sum3 = 0;
    #pragma omp simd reduction(+:sum3)
    for (int i = 0; i < n; i++) {
        sum3 += arr3[i];
    }
    printf("Checksum 3: %d\n", sum3);
    
    /* Test 4: Runtime collapse */
    printf("\nTest 4: Runtime collapse with volatile bounds\n");
    test_runtime_collapse(arr4, 256, 256);
    
    /* Final verification */
    int final_sum = 0;
    #pragma omp simd reduction(+:final_sum)
    for (int i = 0; i < 256*256; i++) {
        final_sum += arr4[i];
    }
    printf("Final checksum: %d\n", final_sum);
    
    /* Force data dependencies */
    volatile int dummy = g_checksum + sum3 + final_sum;
    printf("Total dummy: %d\n", dummy);
    
    return 0;
}
