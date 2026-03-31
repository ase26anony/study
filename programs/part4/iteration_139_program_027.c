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
                private(i) shared(arr, n)
    for (i = 0; i < n; i++) {
        arr[i] = (i + pid) * 2;
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
    volatile int vol_bound = m + getpid() % 16;
    
    /* Allocate device memory explicitly */
    float *dev_ptr = (float *)omp_target_alloc(m * sizeof(float), 0);
    if (!dev_ptr) return;
    
    #pragma omp target is_device_ptr(dev_ptr) map(tofrom: results[0:m]) \
                device(ancestor:1) if(0)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                reduction(+:sum) collapse(2)
    for (i = 0; i < vol_bound; i++) {
        for (j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < m) {
                dev_ptr[idx] = (float)(i * j) / (float)(pid + 1);
                sum += dev_ptr[idx];
            }
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
    int *dev_data = (int *)omp_target_alloc(rows * cols * sizeof(int), 0);
    if (!dev_data) return;
    
    /* Initialize device data */
    #pragma omp target is_device_ptr(dev_data) map(to: rows, cols)
    #pragma omp teams
    {
        #pragma omp distribute
        for (i = 0; i < rows; i++) {
            #pragma omp parallel
            #pragma omp taskloop simd
            for (j = 0; j < cols; j++) {
                int idx = i * cols + j;
                dev_data[idx] = (i << 8) | (j & 0xFF);
            }
        }
    }
    
    /* Process with SIMD */
    #pragma omp target map(tofrom: data[0:rows*cols]) is_device_ptr(dev_data) \
                if(getpid() % 3 == 0) device(simd:2)
    #pragma omp teams distribute simd collapse(2) \
                dist_schedule(static, 8)
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            data[idx] = dev_data[idx] * 3 + (i + j);
        }
    }
    
    /* Verify and compute checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < rows * cols; i++) {
        g_checksum += data[i];
    }
    
    omp_target_free(dev_data, 0);
}

/* Helper function with runtime-dependent bounds */
void test_runtime_bounds(int *arr, int base_size) {
    int dynamic_size = base_size + (getpid() % 128);
    int i;
    
    #pragma omp target map(tofrom: arr[0:dynamic_size]) \
                if(dynamic_size > base_size) device(ancestor:2)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:guided) num_teams(4)
    for (i = 0; i < dynamic_size; i++) {
        arr[i] = arr[i] * 2 + i;
    }
    
    /* Accumulate results */
    int local_sum = 0;
    #pragma omp simd reduction(+:local_sum)
    for (i = 0; i < dynamic_size; i++) {
        local_sum += arr[i];
    }
    
    #pragma omp atomic
    g_checksum += local_sum;
}

int main(int argc, char **argv) {
    int *array1 = (int *)malloc(SIZE * sizeof(int));
    float *array2 = (float *)malloc(SIZE * sizeof(float));
    int *array3 = (int *)malloc(SIZE * 2 * sizeof(int));
    int *array4 = (int *)malloc(SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    #pragma omp simd
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i;
        array2[i] = (float)i / 10.0f;
        array4[i] = i * 2;
    }
    
    #pragma omp simd
    for (int i = 0; i < SIZE * 2; i++) {
        array3[i] = i % 256;
    }
    
    printf("Starting OpenMP SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT transformation */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(array1, SIZE);
    
    /* Test 2: SIMD with reduction and device pointers */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd(array2, SIZE);
    
    /* Test 3: Complex nesting */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(array3, 32, 64);  /* 32x64 = 2048 elements */
    
    /* Test 4: Runtime bounds */
    printf("Test 4: Runtime-dependent bounds\n");
    test_runtime_bounds(array4, SIZE / 2);
    
    /* Final verification */
    printf("Final checksum: %d\n", g_checksum);
    
    /* Simple validation */
    int verify_sum = 0;
    #pragma omp parallel for simd reduction(+:verify_sum)
    for (int i = 0; i < SIZE; i++) {
        verify_sum += array1[i] + (int)array2[i] + array4[i];
    }
    
    #pragma omp parallel for simd reduction(+:verify_sum)
    for (int i = 0; i < SIZE * 2; i++) {
        verify_sum += array3[i];
    }
    
    printf("Verification sum: %d\n", verify_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
