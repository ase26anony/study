/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define N 1024
#define M 512
#define CHUNK_SIZE 16

/* Global variables to prevent optimization */
volatile int g_volatile_counter = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_target_teams_distribute_parallel_for_simd(int *arr, int size, int base) {
    volatile int vol_bound = size;
    int local_sum = 0;
    
    /* Use device clause with simd:1 to potentially trigger SIMT path */
    #pragma omp target map(tofrom: local_sum) map(to: arr[0:size]) \
                      device(simd:1) if(0) num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static, CHUNK_SIZE) reduction(+:local_sum)
    for (int i = 0; i < vol_bound; i++) {
        arr[i] = base + i * 2;
        local_sum += arr[i];
    }
    
    g_volatile_counter++;
    g_checksum += local_sum;
    printf("Test1 sum: %d\n", local_sum);
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_target_teams_distribute_simd(float *results, int size, int pid) {
    volatile int start = pid % 64;
    volatile int end = size;
    float sum = 0.0f;
    
    /* Use ancestor device clause which might affect SIMT decision */
    #pragma omp target map(tofrom: sum) map(from: results[0:size]) \
                      device(ancestor:1) if(pid > 0)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                reduction(+:sum) num_teams(4)
    for (int i = start; i < end; i += 2) {
        results[i] = (float)i * 3.14f / (pid + 1);
        sum += results[i];
    }
    
    g_volatile_counter++;
    g_checksum += (int)sum;
    printf("Test2 sum: %.2f\n", sum);
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *arr, float *results, int size) {
    int *dev_ptr = NULL;
    int host_arr[64];
    volatile int vol_size = size < 64 ? size : 64;
    
    /* Initialize host array */
    for (int i = 0; i < 64; i++) {
        host_arr[i] = i * 3;
    }
    
    /* Allocate device memory explicitly */
    dev_ptr = (int *)omp_target_alloc(64 * sizeof(int), omp_get_default_device());
    if (!dev_ptr) {
        printf("Device allocation failed\n");
        return;
    }
    
    /* Copy data to device */
    #pragma omp target data map(to: host_arr[0:64]) \
                           map(tofrom: arr[0:size]) \
                           use_device_ptr(dev_ptr)
    {
        /* Complex target region with multiple constructs */
        #pragma omp target if(size > 256) device(simd:1) \
                          map(to: host_arr[0:64]) map(tofrom: arr[0:size]) \
                          is_device_ptr(dev_ptr)
        {
            /* Copy from host_arr to device pointer */
            #pragma omp teams distribute simd
            for (int i = 0; i < vol_size; i++) {
                dev_ptr[i] = host_arr[i] + 1;
            }
            
            /* Nested loop with collapse */
            #pragma omp teams distribute parallel for simd collapse(2) \
                        num_teams(2) thread_limit(32)
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 4; j++) {
                    int idx = i * 4 + j;
                    if (idx < size) {
                        arr[idx] = dev_ptr[idx % 64] * (i + 1);
                    }
                }
            }
            
            /* Taskloop simd inside teams region */
            #pragma omp teams
            {
                #pragma omp taskloop simd
                for (int i = 32; i < vol_size; i++) {
                    results[i] = (float)arr[i] / 2.0f;
                }
            }
        }
    }
    
    /* Verify some results */
    int verify_sum = 0;
    for (int i = 0; i < (size < 64 ? size : 64); i++) {
        verify_sum += arr[i];
    }
    
    g_volatile_counter++;
    g_checksum += verify_sum;
    printf("Test3 verify sum: %d\n", verify_sum);
    
    /* Free device memory */
    omp_target_free(dev_ptr, omp_get_default_device());
}

/* Additional test with runtime bounds and collapse */
void test_runtime_collapse(int *arr, int size, int seed) {
    volatile int outer_bound = (seed % 8) + 4;
    volatile int inner_bound = (seed % 16) + 8;
    int total = 0;
    
    /* Target with if clause that might be false at runtime */
    #pragma omp target map(tofrom: total) map(to: arr[0:size]) \
                      if(seed % 2) device(simd:1)
    #pragma omp teams distribute parallel for simd collapse(2) \
                reduction(+:total) num_teams(outer_bound)
    for (int i = 0; i < outer_bound; i++) {
        for (int j = 0; j < inner_bound; j++) {
            int idx = i * inner_bound + j;
            if (idx < size) {
                arr[idx] = (i + 1) * (j + 1) * seed;
                total += arr[idx];
            }
        }
    }
    
    g_volatile_counter++;
    g_checksum += total;
    printf("Test4 total: %d\n", total);
}

int main(int argc, char *argv[]) {
    int *arr = (int *)malloc(N * sizeof(int));
    float *results = (float *)malloc(M * sizeof(float));
    int pid = getpid();
    
    if (!arr || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) arr[i] = 0;
    for (int i = 0; i < M; i++) results[i] = 0.0f;
    
    printf("PID: %d\n", pid);
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMD with static scheduling */
    test_target_teams_distribute_parallel_for_simd(arr, N, pid);
    
    /* Test 2: Teams distribute simd with reduction */
    test_target_teams_distribute_simd(results, M, pid);
    
    /* Test 3: Complex nesting */
    test_complex_nesting(arr, results, N);
    
    /* Test 4: Runtime collapse */
    test_runtime_collapse(arr, N, pid);
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += arr[i];
    }
    
    printf("Final array sum: %d\n", final_sum);
    printf("Global checksum: %d\n", g_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    free(arr);
    free(results);
    
    return 0;
}
