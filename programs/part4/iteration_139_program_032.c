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
    volatile int v_n = n;
    int device_id = omp_get_default_device();
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:v_n]) map(to: base) \
                     num_teams(4) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                     collapse(2) shared(arr)
    for (int i = 0; i < v_n; i += BLOCK) {
        for (int j = 0; j < BLOCK && (i + j) < v_n; j++) {
            int idx = i + j;
            arr[idx] = idx * base + (idx % 16);
        }
    }
    
    /* Store results for verification */
    for (int i = 0; i < (n < SIZE ? n : SIZE); i++) {
        g_results[i] += arr[i];
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_simt_wrapper_2(float *data, int m, int n) {
    volatile int v_m = m;
    volatile int v_n = n;
    float sum = 0.0f;
    
    /* Allocate device memory explicitly */
    float *dev_ptr = (float *)omp_target_alloc(v_m * v_n * sizeof(float), 
                                               omp_get_default_device());
    if (!dev_ptr) return;
    
    #pragma omp target device(ancestor:1) is_device_ptr(dev_ptr) \
                     map(to: v_m, v_n) reduction(+:sum)
    #pragma omp teams distribute simd dist_schedule(static, 16) collapse(2) \
                     reduction(+:sum)
    for (int i = 0; i < v_m; i++) {
        for (int j = 0; j < v_n; j++) {
            int idx = i * v_n + j;
            float val = (i * 1.5f + j * 0.5f) / (v_m + v_n);
            dev_ptr[idx] = val;
            sum += val;
        }
    }
    
    /* Copy back and accumulate */
    #pragma omp target is_device_ptr(dev_ptr) map(from: data[0:v_m*v_n])
    #pragma omp teams distribute parallel for simd
    for (int i = 0; i < v_m * v_n; i++) {
        data[i] = dev_ptr[i];
    }
    
    g_volatile_counter = (int)(sum * 1000);
    omp_target_free(dev_ptr, omp_get_default_device());
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *out, int rows, int cols, int seed) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    int local_sum = 0;
    
    #pragma omp target map(to: v_rows, v_cols, seed) map(tofrom: out[0:v_rows*v_cols]) \
                     map(tofrom: local_sum)
    {
        #pragma omp teams num_teams(8) thread_limit(64) reduction(+:local_sum)
        {
            #pragma omp distribute
            for (int r = 0; r < v_rows; r++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(1) grainsize(8) \
                                 reduction(+:local_sum)
                    for (int c = 0; c < v_cols; c++) {
                        int idx = r * v_cols + c;
                        int val = (r * 37 + c * 13 + seed) % 256;
                        out[idx] = val;
                        local_sum += val;
                        
                        /* Nested SIMD operation */
                        #pragma omp simd
                        for (int k = 0; k < 4; k++) {
                            out[idx] += k * (r % 16);
                        }
                    }
                }
            }
        }
    }
    
    /* Update global results */
    for (int i = 0; i < v_rows * v_cols && i < SIZE; i++) {
        g_results[i] ^= out[i];
    }
    g_volatile_counter += local_sum;
}

/* Helper function with runtime-dependent bounds */
void test_dynamic_bounds(int pid) {
    int size = 256 + (pid % 128);
    int *dyn_arr = (int *)malloc(size * sizeof(int));
    if (!dyn_arr) return;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        dyn_arr[i] = i * 2;
    }
    
    #pragma omp target map(tofrom: dyn_arr[0:size]) device(simd:1)
    #pragma omp teams distribute parallel for simd schedule(static, 8)
    for (int i = 0; i < size; i++) {
        dyn_arr[i] = dyn_arr[i] * 3 + (i % 32);
    }
    
    /* Accumulate to global */
    for (int i = 0; i < size && i < SIZE; i++) {
        g_results[i] += dyn_arr[i];
    }
    
    free(dyn_arr);
}

int main(int argc, char **argv) {
    int pid = getpid();
    printf("Test PID: %d\n", pid);
    
    /* Initialize test data */
    int arr1[SIZE];
    float arr2[SIZE];
    int arr3[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = 0;
        arr2[i] = 0.0f;
        arr3[i] = i;
        g_results[i] = 0;
    }
    
    /* Test 1: Basic SIMT wrapper */
    printf("Running test 1...\n");
    test_simt_wrapper_1(arr1, SIZE, pid % 100);
    
    /* Test 2: Reduction with device pointer */
    printf("Running test 2...\n");
    test_simt_wrapper_2(arr2, 32, 32);
    
    /* Test 3: Complex nesting */
    printf("Running test 3...\n");
    test_simt_wrapper_3(arr3, 16, 16, pid);
    
    /* Test 4: Dynamic bounds */
    printf("Running test 4...\n");
    test_dynamic_bounds(pid);
    
    /* Verify results */
    long checksum1 = 0, checksum2 = 0, checksum3 = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum1 += arr1[i];
        checksum2 += (int)(arr2[i] * 1000);
        checksum3 += arr3[i];
    }
    
    long global_checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        global_checksum += g_results[i];
    }
    
    printf("Checksum 1: %ld\n", checksum1);
    printf("Checksum 2: %ld\n", checksum2);
    printf("Checksum 3: %ld\n", checksum3);
    printf("Global checksum: %ld\n", global_checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Final verification */
    int success = 1;
    if (checksum1 == 0 && checksum2 == 0 && checksum3 == 0) {
        printf("ERROR: All checksums zero!\n");
        success = 0;
    }
    
    if (global_checksum == 0) {
        printf("ERROR: Global checksum zero!\n");
        success = 0;
    }
    
    printf("Test %s\n", success ? "PASSED" : "FAILED");
    return success ? 0 : 1;
}
