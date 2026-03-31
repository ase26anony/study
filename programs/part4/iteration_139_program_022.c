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
void test_target_teams_distribute_parallel_for_simd(int *arr, int n, int base) {
    volatile int vol_bound = n + (getpid() % 16); /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(to: vol_bound, base) \
                       num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                       num_threads(64)
    for (int i = 0; i < vol_bound; i++) {
        arr[i] = base + i * 2 + (i % 16);
        g_volatile_counter++; /* Force side effect */
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd_reduction(float *data, int n, float *sum_ptr) {
    volatile int start = (getpid() % 8) * BLOCK;
    volatile int end = start + n;
    float local_sum = 0.0f;
    
    /* Allocate device memory explicitly */
    float *dev_data = (float *)omp_target_alloc(n * sizeof(float), 
                                               omp_get_default_device());
    if (!dev_data) return;
    
    #pragma omp target device(ancestor:1) is_device_ptr(dev_data) \
                       map(to: start, end) map(from: local_sum) \
                       dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:local_sum)
    for (int i = start; i < end && i < SIZE; i++) {
        dev_data[i - start] = (float)i * 1.5f;
        local_sum += dev_data[i - start];
        g_float_results[i] = dev_data[i - start]; /* Store to global */
    }
    
    *sum_ptr = local_sum;
    
    /* Copy back and free */
    #pragma omp target is_device_ptr(dev_data) map(tofrom: data[0:n])
    #pragma omp teams distribute parallel for simd
    for (int i = 0; i < n; i++) {
        data[i] = dev_data[i];
    }
    
    omp_target_free(dev_data, omp_get_default_device());
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *arr1, int *arr2, int n, int collapse_factor) {
    volatile int dynamic_bound = n + (getpid() % 32);
    
    #pragma omp target if(1) map(tofrom: arr1[0:n], arr2[0:n]) \
                       map(to: dynamic_bound, collapse_factor) \
                       device(0) /* Force device selection */
    {
        #pragma omp teams num_teams(2) thread_limit(64)
        {
            int team_id = omp_get_team_num();
            
            #pragma omp distribute dist_schedule(static)
            for (int i = 0; i < dynamic_bound; i += BLOCK) {
                int block_end = (i + BLOCK < dynamic_bound) ? i + BLOCK : dynamic_bound;
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(2) grainsize(8) \
                                   if(team_id == 0) /* Conditional clause */
                    for (int j = i; j < block_end; j++) {
                        for (int k = 0; k < collapse_factor; k++) {
                            int idx = j * collapse_factor + k;
                            if (idx < n) {
                                arr1[idx] = (team_id * 1000) + j + k;
                                arr2[idx] = arr1[idx] * 2;
                                g_results[idx] = arr1[idx] + arr2[idx];
                            }
                        }
                    }
                }
            }
        }
    }
}

/* Helper function with volatile arguments */
void test_volatile_bounds(volatile int *bounds, int *output, int count) {
    #pragma omp target map(to: bounds[0:2]) map(from: output[0:count]) \
                       device(simd:1) if(0)
    #pragma omp teams distribute parallel for simd \
                       schedule(static, 16) num_teams(bounds[1] - bounds[0] + 7 / 8)
    for (int i = bounds[0]; i < bounds[1]; i++) {
        if (i < count) {
            output[i] = i * i - i;
            g_volatile_counter += output[i] % 7;
        }
    }
}

int main(int argc, char **argv) {
    int arr1[SIZE], arr2[SIZE];
    float float_arr[SIZE];
    float sum = 0.0f;
    int checksum1 = 0, checksum2 = 0, checksum3 = 0;
    volatile int bounds[2] = {0, SIZE/2};
    
    /* Initialize with runtime-dependent values */
    int seed = getpid() + (argc > 1 ? atoi(argv[1]) : 0);
    srand(seed);
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = 0;
        float_arr[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMD with schedule(simd:static) */
    test_target_teams_distribute_parallel_for_simd(arr1, SIZE, 1000);
    for (int i = 0; i < SIZE; i++) {
        checksum1 += arr1[i];
    }
    printf("Test 1 checksum: %d\n", checksum1);
    
    /* Test 2: Reduction with explicit device memory */
    test_target_teams_distribute_simd_reduction(float_arr, SIZE/2, &sum);
    for (int i = 0; i < SIZE/2; i++) {
        checksum2 += (int)(float_arr[i] * 100);
    }
    printf("Test 2 sum: %.2f, checksum: %d\n", sum, checksum2);
    
    /* Test 3: Complex nesting */
    bounds[1] = SIZE/4 + (seed % 64);
    test_complex_nesting(arr1, arr2, SIZE/2, 2);
    for (int i = 0; i < SIZE/2; i++) {
        checksum3 += arr1[i] + arr2[i];
    }
    printf("Test 3 checksum: %d\n", checksum3);
    
    /* Test 4: Volatile bounds */
    int output[SIZE/2] = {0};
    bounds[0] = 10;
    bounds[1] = 10 + SIZE/4;
    test_volatile_bounds(bounds, output, SIZE/2);
    
    int checksum4 = 0;
    for (int i = 0; i < SIZE/4; i++) {
        checksum4 += output[i];
    }
    printf("Test 4 checksum: %d\n", checksum4);
    
    /* Final verification */
    int final_checksum = checksum1 + checksum2 + checksum3 + checksum4;
    printf("Final combined checksum: %d\n", final_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Print some global results for verification */
    printf("Sample global results[0:5]: ");
    for (int i = 0; i < 5; i++) {
        printf("%d ", g_results[i]);
    }
    printf("\n");
    
    return 0;
}
