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
    int local_sum = 0;
    int pid_mod = getpid() % 100;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(tofrom: local_sum) map(to: arr[0:n]) \
                      device(ancestor:1) if(pid_mod > 50)
    #pragma omp teams distribute parallel for simd \
                num_teams(4) thread_limit(32) schedule(simd:static) \
                reduction(+:local_sum)
    for (int i = 0; i < n; i++) {
        local_sum += arr[i] * (i % 16);
    }
    
    /* Nested loop with collapse to increase complexity */
    volatile int v_bound = n / 2 + pid_mod;
    #pragma omp target map(tofrom: result[0:v_bound]) map(to: arr[0:n]) \
                      device(simd:1) if(0)
    #pragma omp teams distribute parallel for simd collapse(2) \
                dist_schedule(static, 16) schedule(static, 8)
    for (int i = 0; i < v_bound; i++) {
        for (int j = 0; j < 8; j++) {
            result[i] = arr[i * 8 + j] + local_sum;
        }
    }
    
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with reduction */
void test_simt_wrapper_2(float *data, int n, float *output) {
    float sum = 0.0f;
    int dynamic_bound = n + (getpid() % 32);
    
    /* Mixed device clauses to explore different paths */
    #pragma omp target map(tofrom: sum) map(to: data[0:n]) \
                      device(ancestor:2) if(1)
    #pragma omp teams distribute simd \
                dist_schedule(static, 32) reduction(+:sum)
    for (int i = 0; i < dynamic_bound; i++) {
        sum += data[i] * 0.5f;
    }
    
    /* Additional loop with volatile bounds */
    volatile int v_iter = g_volatile_bound ? g_volatile_bound : 16;
    #pragma omp target map(tofrom: output[0:n]) map(to: data[0:n]) \
                      device(simd:2) if(v_iter > 8)
    #pragma omp teams distribute parallel for simd \
                num_teams(v_iter) schedule(static, 4)
    for (int i = 0; i < n; i += 2) {
        output[i] = data[i] * sum;
        output[i + 1] = data[i + 1] / (sum + 1.0f);
    }
    
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *a, int *b, int *c, int n) {
    int pid = getpid();
    
    /* Complex nesting that might trigger special handling */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                      device(ancestor: pid % 3) if(pid % 2)
    {
        #pragma omp teams num_teams(2) thread_limit(64)
        {
            #pragma omp distribute dist_schedule(static, 8)
            for (int i = 0; i < n; i += BLOCK) {
                int limit = (i + BLOCK) < n ? (i + BLOCK) : n;
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd simdlen(8) \
                                if(limit - i > 32) nogroup
                    for (int j = i; j < limit; j++) {
                        c[j] = a[j] * 2 + b[j] / 3;
                    }
                }
            }
        }
    }
    
    /* Second region with different construct */
    volatile int v_n = n;
    #pragma omp target map(tofrom: c[0:v_n]) \
                      device(simd:3) if(v_n > 256)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:guided) num_teams(8)
    for (int i = 0; i < v_n; i++) {
        c[i] += (i % 16);
    }
    
    /* Compute checksum */
    int local_sum = 0;
    for (int i = 0; i < n && i < 128; i++) {
        local_sum += c[i];
    }
    g_checksum += local_sum;
}

/* Function 4: Using omp_target_alloc with is_device_ptr */
void test_device_ptr_simt(int n) {
    size_t size = n * sizeof(int);
    int *device_arr = (int *)omp_target_alloc(size, 0);
    int *host_arr = (int *)malloc(size);
    int *result_arr = (int *)malloc(size);
    
    if (!device_arr || !host_arr || !result_arr) {
        fprintf(stderr, "Allocation failed\n");
        free(host_arr);
        free(result_arr);
        if (device_arr) omp_target_free(device_arr, 0);
        return;
    }
    
    /* Initialize host data */
    for (int i = 0; i < n; i++) {
        host_arr[i] = i * 2 + (getpid() % 7);
    }
    
    /* Copy to device */
    omp_target_memcpy(device_arr, host_arr, size, 0, 0, 0, 0);
    
    /* Use is_device_ptr to force complex data environment */
    #pragma omp target is_device_ptr(device_arr) map(tofrom: result_arr[0:n]) \
                      device(ancestor:4) if(n > 128)
    #pragma omp teams distribute parallel for simd \
                num_teams((n + 63)/64) schedule(static, 16)
    for (int i = 0; i < n; i++) {
        result_arr[i] = device_arr[i] * 3 - (i % 11);
    }
    
    /* Verify results */
    int check = 0;
    for (int i = 0; i < n && i < 64; i++) {
        check += result_arr[i];
    }
    g_checksum += check;
    
    /* Cleanup */
    free(host_arr);
    free(result_arr);
    omp_target_free(device_arr, 0);
}

int main(int argc, char **argv) {
    int n = SIZE;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 64) n = 64;
    if (n > 4096) n = 4096;
    
    g_volatile_bound = (getpid() % 64);
    
    /* Allocate test arrays */
    int *arr1 = (int *)malloc(n * sizeof(int));
    int *arr2 = (int *)malloc(n * sizeof(int));
    int *arr3 = (int *)malloc(n * sizeof(int));
    float *farr1 = (float *)malloc(n * sizeof(float));
    float *farr2 = (float *)malloc(n * sizeof(float));
    int *results = (int *)malloc(n * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !farr1 || !farr2 || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 3 - 7;
        arr2[i] = i * 5 + 11;
        arr3[i] = i * 2 - 3;
        farr1[i] = (float)i * 0.25f;
        farr2[i] = (float)i * 0.75f;
        results[i] = 0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("Array size: %d, PID: %d\n", n, getpid());
    
    /* Execute test functions */
    test_simt_wrapper_1(arr1, n, results);
    printf("Test 1 completed, checksum: %d\n", g_checksum);
    
    test_simt_wrapper_2(farr1, n, farr2);
    printf("Test 2 completed, checksum: %d\n", g_checksum);
    
    test_simt_wrapper_3(arr1, arr2, arr3, n);
    printf("Test 3 completed, checksum: %d\n", g_checksum);
    
    test_device_ptr_simt(n / 2);
    printf("Test 4 completed, checksum: %d\n", g_checksum);
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < n && i < 128; i++) {
        final_check += arr3[i] + results[i];
    }
    g_checksum += final_check;
    
    printf("Final checksum: %d\n", g_checksum);
    printf("All tests completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    free(results);
    
    return 0;
}
