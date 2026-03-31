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
    volatile int v_bound = n + (getpid() % 16); /* Runtime-dependent bound */
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(to: v_bound, base) \
                     num_teams(8) thread_limit(128)
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
    volatile int end = n + seed;
    float sum = 0.0f;
    
    /* Allocate device memory explicitly */
    float *d_ptr = (float *)omp_target_alloc(n * sizeof(float), 0);
    if (!d_ptr) return;
    
    #pragma omp target device(ancestor:1) is_device_ptr(d_ptr) \
                     map(to: start, end) map(from: sum) \
                     dist_schedule(static, 16)
    #pragma omp teams distribute simd reduction(+:sum)
    for (int i = start; i < end; i++) {
        float val = (i % 7) * 0.5f;
        d_ptr[i - start] = val;
        sum += val;
    }
    
    /* Copy back and accumulate */
    #pragma omp target teams distribute parallel for simd \
                     map(tofrom: results[0:n]) is_device_ptr(d_ptr)
    for (int i = 0; i < n; i++) {
        results[i] += d_ptr[i];
    }
    
    omp_target_free(d_ptr, 0);
    
    /* Store sum in global checksum */
    #pragma omp atomic
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *arr, int n, int m) {
    volatile int outer_bound = n;
    volatile int inner_bound = m;
    
    #pragma omp target if(1) map(tofrom: arr[0:n*m]) \
                     map(to: outer_bound, inner_bound) \
                     device(0)
    #pragma omp teams num_teams(4) thread_limit(32)
    {
        #pragma omp distribute
        for (int i = 0; i < outer_bound; i++) {
            #pragma omp parallel
            {
                #pragma omp taskloop simd collapse(2) grainsize(8) \
                                 if(0) nogroup
                for (int j = 0; j < inner_bound; j++) {
                    for (int k = 0; k < 4; k++) {
                        int idx = i * inner_bound + j;
                        arr[idx] = (i << 16) | (j << 8) | k;
                    }
                }
            }
        }
    }
}

/* Function 4: Mixed constructs with runtime bounds */
void test_mixed_constructs(double *data, int size, int offset) {
    volatile int dyn_size = size + (offset % 32);
    int chunk = 16;
    
    #pragma omp target if(offset % 2) device(simd:2) \
                     map(tofrom: data[0:size]) map(to: dyn_size, chunk)
    #pragma omp teams distribute parallel for simd \
                     collapse(2) schedule(static, chunk)
    for (int i = 0; i < dyn_size; i += chunk) {
        for (int j = 0; j < chunk && (i + j) < dyn_size; j++) {
            int idx = i + j;
            data[idx] = (idx * 3.14159) / (offset + 1);
        }
    }
}

/* Helper function to verify results */
int verify_array(int *arr, int n, int expected_base) {
    int local_sum = 0;
    for (int i = 0; i < n; i++) {
        local_sum += arr[i];
    }
    return local_sum;
}

float verify_float_array(float *arr, int n) {
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(int argc, char **argv) {
    int n = SIZE;
    int m = BLOCK;
    
    /* Initialize arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * m * sizeof(int));
    float *float_arr = (float *)malloc(n * sizeof(float));
    double *double_arr = (double *)malloc(n * sizeof(double));
    
    if (!arr1 || !arr2 || !float_arr || !double_arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < n; i++) {
        arr1[i] = 0;
        float_arr[i] = 0.0f;
        double_arr[i] = 0.0;
    }
    for (int i = 0; i < n * m; i++) {
        arr2[i] = 0;
    }
    
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    int seed = getpid() % 256;
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d, Base: %d, Seed: %d\n", getpid(), base, seed);
    
    /* Test 1: Basic SIMT transformation */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr1, n, base);
    int sum1 = verify_array(arr1, n, base);
    printf("Checksum 1: %d\n", sum1);
    
    /* Test 2: SIMD with reduction and device pointers */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd_reduction(float_arr, n, seed);
    float sum2 = verify_float_array(float_arr, n);
    printf("Checksum 2: %f\n", sum2);
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(arr2, n/16, m);
    int sum3 = verify_array(arr2, n/16 * m, 0);
    printf("Checksum 3: %d\n", sum3);
    
    /* Test 4: Mixed constructs */
    printf("\nTest 4: Mixed constructs with collapse\n");
    test_mixed_constructs(double_arr, n, seed);
    double sum4 = 0.0;
    for (int i = 0; i < n; i++) {
        sum4 += double_arr[i];
    }
    printf("Checksum 4: %f\n", sum4);
    
    /* Print global counters */
    printf("\nGlobal counters:\n");
    printf("g_volatile_counter: %d\n", g_volatile_counter);
    printf("g_checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(float_arr);
    free(double_arr);
    
    printf("\nAll tests completed.\n");
    return 0;
}
