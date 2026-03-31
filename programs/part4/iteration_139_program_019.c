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
    int i;
    int local_sum = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int vol_n = n;
    
    /* Target region with if clause and device clause to trigger SIMT path */
    #pragma omp target map(tofrom: local_sum) map(to: arr[0:vol_n]) \
                      if(0) device(simd:1) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static, 32) reduction(+:local_sum)
    for (i = 0; i < vol_n; i++) {
        local_sum += arr[i] * 2;
    }
    
    /* Nested loop to potentially trigger collapse handling */
    #pragma omp target map(tofrom: result[0:BLOCK]) map(to: arr[0:vol_n]) \
                      device(ancestor:1)
    #pragma omp teams distribute parallel for simd collapse(2) \
                num_teams(4)
    for (i = 0; i < BLOCK/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            result[idx] = arr[idx % vol_n] + local_sum;
        }
    }
    
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with reduction */
void test_simt_wrapper_2(float *data, int n, float *output) {
    float sum = 0.0f;
    int runtime_bound = n + (getpid() % 16); /* Make bound runtime-dependent */
    
    /* Complex target region with multiple clauses */
    #pragma omp target map(to: data[0:runtime_bound]) map(from: output[0:n]) \
                      map(tofrom: sum) if(runtime_bound > 0) \
                      device(simd:2)
    #pragma omp teams distribute simd \
                dist_schedule(static, 16) reduction(+:sum) \
                num_teams(runtime_bound / 32 + 1)
    for (int i = 0; i < runtime_bound; i++) {
        sum += data[i];
        output[i] = data[i] * 2.0f;
    }
    
    /* Second loop with different SIMD pattern */
    volatile int vol_idx = 0;
    #pragma omp target map(tofrom: output[0:n]) device(ancestor:2)
    #pragma omp teams distribute parallel for simd \
                schedule(static, 8)
    for (int i = 0; i < n; i += 2) {
        vol_idx = i;
        output[vol_idx] += sum;
        if (i + 1 < n) {
            output[i + 1] -= sum;
        }
    }
    
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *a, int *b, int *c, int n) {
    int *dev_ptr = NULL;
    int dev_size = n * sizeof(int);
    
    /* Allocate device memory explicitly */
    dev_ptr = (int *)omp_target_alloc(dev_size, omp_get_default_device());
    if (!dev_ptr) return;
    
    /* Initialize device memory */
    #pragma omp target teams distribute parallel for simd \
                is_device_ptr(dev_ptr) map(to: a[0:n])
    for (int i = 0; i < n; i++) {
        dev_ptr[i] = a[i];
    }
    
    /* Complex region with taskloop simd */
    #pragma omp target map(to: b[0:n]) map(from: c[0:n]) \
                      if(1) device(simd:3) \
                      is_device_ptr(dev_ptr)
    {
        #pragma omp teams num_teams(4) thread_limit(64)
        {
            #pragma omp distribute
            for (int team = 0; team < 4; team++) {
                int start = team * (n / 4);
                int end = (team == 3) ? n : (team + 1) * (n / 4);
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd \
                                shared(dev_ptr, b, c) \
                                private(start, end) \
                                grainsize(8)
                    for (int i = start; i < end; i++) {
                        c[i] = dev_ptr[i] * b[i] + i;
                    }
                }
            }
        }
    }
    
    /* Verify results on host */
    int check = 0;
    for (int i = 0; i < n; i++) {
        check += c[i];
    }
    g_checksum += check;
    
    omp_target_free(dev_ptr, omp_get_default_device());
}

/* Helper function with runtime bounds */
void test_variable_bounds(int seed) {
    int dynamic_size = 256 + (seed % 128);
    int *temp_arr = (int *)malloc(dynamic_size * sizeof(int));
    int *temp_out = (int *)malloc(dynamic_size * sizeof(int));
    
    if (!temp_arr || !temp_out) {
        free(temp_arr);
        free(temp_out);
        return;
    }
    
    for (int i = 0; i < dynamic_size; i++) {
        temp_arr[i] = i * seed;
    }
    
    /* Target with runtime bounds */
    #pragma omp target map(to: temp_arr[0:dynamic_size]) \
                      map(from: temp_out[0:dynamic_size]) \
                      if(dynamic_size > 300) device(simd:4)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:guided)
    for (int i = 0; i < dynamic_size; i++) {
        temp_out[i] = temp_arr[i] * 3 - seed;
    }
    
    int sum = 0;
    for (int i = 0; i < dynamic_size; i++) {
        sum += temp_out[i];
    }
    g_checksum += sum;
    
    free(temp_arr);
    free(temp_out);
}

int main(int argc, char **argv) {
    int arr[SIZE];
    float farr[SIZE];
    int results[SIZE];
    float fresults[SIZE];
    int other_arr[SIZE];
    
    /* Initialize with non-trivial patterns */
    int seed = getpid();
    srand(seed);
    
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + seed;
        farr[i] = (float)(i * 0.5) + seed * 0.1;
        results[i] = 0;
        fresults[i] = 0.0f;
        other_arr[i] = rand() % 100;
    }
    
    g_volatile_bound = SIZE / 2 + (seed % 64);
    
    printf("Starting SIMT transformation tests...\n");
    printf("Using seed: %d, volatile bound: %d\n", seed, g_volatile_bound);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, g_volatile_bound, results);
    
    /* Test 2: teams distribute simd with reduction */
    printf("Test 2: teams distribute simd with reduction\n");
    test_simt_wrapper_2(farr, SIZE / 2, fresults);
    
    /* Test 3: Complex nesting with device pointers */
    printf("Test 3: Complex nesting with device pointers\n");
    test_simt_wrapper_3(arr, other_arr, results, SIZE / 4);
    
    /* Test 4: Variable bounds */
    printf("Test 4: Variable bounds\n");
    test_variable_bounds(seed);
    
    /* Verify results */
    int final_check = 0;
    for (int i = 0; i < SIZE; i++) {
        final_check += results[i];
        final_check += (int)fresults[i];
    }
    final_check += g_checksum;
    
    printf("\nFinal checksum: %d\n", final_check);
    printf("g_checksum accumulated: %d\n", g_checksum);
    
    /* Additional target region to ensure coverage */
    #pragma omp target map(to: arr[0:64]) device(simd:5) if(0)
    #pragma omp teams distribute parallel for simd \
                num_teams(2) thread_limit(32)
    for (int i = 0; i < 64; i++) {
        arr[i] = arr[i] * 2;
    }
    
    return 0;
}
