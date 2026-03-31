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
    int pid = getpid() % 100;
    
    #pragma omp target map(tofrom: arr[0:n]) if(pid > 50) device(simd:1)
    #pragma omp teams num_teams(8) thread_limit(64)
    #pragma omp distribute parallel for simd schedule(simd:static, 32) \
                private(i) shared(arr)
    for (i = 0; i < n; i++) {
        arr[i] = (i * 3 + pid) % 256;
    }
    
    /* Compute checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < n; i++) {
        g_checksum += arr[i];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_target_teams_distribute_simd(float *results, int m) {
    int i, j;
    float sum = 0.0f;
    volatile int vol_bound = m;
    
    /* Allocate device memory explicitly */
    float *dev_ptr = (float *)omp_target_alloc(m * sizeof(float), 0);
    
    #pragma omp target is_device_ptr(dev_ptr) map(tofrom: results[0:m]) \
                device(ancestor:1) if(0)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                reduction(+:sum) collapse(2)
    for (i = 0; i < vol_bound; i++) {
        for (j = 0; j < BLOCK; j++) {
            float val = (i * 0.5f + j * 1.5f);
            dev_ptr[i * BLOCK + j] = val;
            sum += val;
        }
    }
    
    /* Copy back and accumulate */
    #pragma omp target is_device_ptr(dev_ptr) map(from: results[0:m])
    #pragma omp teams distribute parallel for simd
    for (i = 0; i < m; i++) {
        results[i] = dev_ptr[i];
    }
    
    omp_target_free(dev_ptr, 0);
    
    /* Update global checksum */
    #pragma omp atomic
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int rows, int cols) {
    int i, j;
    int runtime_bound = getpid() % rows + 1;
    
    /* Use volatile to prevent constant propagation */
    volatile int *vol_data = data;
    
    #pragma omp target map(tofrom: data[0:rows*cols]) if(runtime_bound > 10) \
                device(simd:2)
    #pragma omp teams num_teams(4)
    {
        #pragma omp distribute
        for (i = 0; i < runtime_bound; i++) {
            #pragma omp parallel
            {
                #pragma omp taskloop simd collapse(2) grainsize(8) \
                            shared(vol_data) private(j)
                for (i = 0; i < rows; i++) {
                    for (j = 0; j < cols; j++) {
                        int idx = i * cols + j;
                        vol_data[idx] = (i * 17 + j * 23 + runtime_bound) % 1024;
                    }
                }
            }
        }
    }
    
    /* Nested loop with SIMD */
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: data[0:rows*cols/2]) if(1) \
                schedule(simd:guided)
    for (i = 0; i < rows * cols / 2; i++) {
        data[i] = data[i] * 2 - data[rows * cols - i - 1];
    }
}

/* Helper function with runtime-dependent bounds */
void test_variable_bounds(double *output, int max_iter) {
    int i;
    int dynamic_bound = (getpid() % max_iter) + 100;
    g_volatile_bound = dynamic_bound;
    
    /* Device pointer with complex mapping */
    double *dev_buf = (double *)omp_target_alloc(dynamic_bound * sizeof(double), 0);
    
    #pragma omp target is_device_ptr(dev_buf) map(from: output[0:dynamic_bound]) \
                if(g_volatile_bound > 50) device(simd:3)
    #pragma omp teams distribute parallel for simd \
                num_threads(32) schedule(simd:dynamic, 8)
    for (i = 0; i < g_volatile_bound; i++) {
        dev_buf[i] = (i * 3.14159) / (i + 1);
    }
    
    /* Copy with SIMD */
    #pragma omp target teams distribute simd \
                is_device_ptr(dev_buf) map(tofrom: output[0:dynamic_bound])
    for (i = 0; i < dynamic_bound; i++) {
        output[i] = dev_buf[i] * 2.0;
    }
    
    omp_target_free(dev_buf, 0);
}

int main(int argc, char *argv[]) {
    int arr[SIZE];
    float results[SIZE/2];
    int matrix[64][64];
    double dynamic_output[500];
    
    int i, j;
    
    /* Initialize with non-zero values */
    for (i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    for (i = 0; i < SIZE/2; i++) {
        results[i] = 0.0f;
    }
    
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
    }
    
    for (i = 0; i < 500; i++) {
        dynamic_output[i] = 0.0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT transformation */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr, SIZE);
    printf("  Checksum after test 1: %d\n", g_checksum);
    
    /* Test 2: SIMD with reduction and device pointers */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd(results, SIZE/2);
    printf("  Checksum after test 2: %d\n", g_checksum);
    
    /* Test 3: Complex nesting */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(&matrix[0][0], 64, 64);
    
    /* Verify matrix computation */
    int matrix_sum = 0;
    #pragma omp simd reduction(+:matrix_sum) collapse(2)
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            matrix_sum += matrix[i][j];
        }
    }
    printf("  Matrix checksum: %d\n", matrix_sum);
    
    /* Test 4: Variable bounds with device allocation */
    printf("Test 4: Variable bounds with device pointers\n");
    test_variable_bounds(dynamic_output, 400);
    
    /* Verify dynamic output */
    double dyn_sum = 0.0;
    #pragma omp simd reduction(+:dyn_sum)
    for (i = 0; i < 200; i++) {
        dyn_sum += dynamic_output[i];
    }
    printf("  Dynamic output sum: %f\n", dyn_sum);
    
    printf("All tests completed. Total checksum: %d\n", g_checksum);
    
    /* Final verification */
    int final_check = 0;
    #pragma omp target teams distribute parallel for simd \
                map(tofrom: arr[0:SIZE]) reduction(+:final_check) \
                if(argc > 1) device(simd:4)
    for (i = 0; i < SIZE; i++) {
        final_check += arr[i] % 128;
    }
    
    printf("Final verification check: %d\n", final_check);
    
    return 0;
}
