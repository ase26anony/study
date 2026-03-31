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
    
    #pragma omp target if(pid > 50) device(simd:1) map(tofrom: arr[0:n]) map(to: n, pid)
    #pragma omp teams distribute parallel for simd schedule(simd:static) num_teams(4) thread_limit(128)
    for (i = 0; i < n; i++) {
        arr[i] = (i + pid) * 2;
    }
    
    /* Compute checksum */
    #pragma omp simd reduction(+:g_checksum)
    for (i = 0; i < BLOCK; i++) {
        g_checksum += arr[i];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_target_teams_distribute_simd(float *results, int m) {
    int i, j;
    float sum = 0.0f;
    volatile int vol_bound = m + getpid() % 32;
    
    #pragma omp target if(0) device(ancestor:1) map(tofrom: results[0:m], sum) map(to: vol_bound)
    #pragma omp teams distribute simd dist_schedule(static, 16) reduction(+:sum) num_teams(8)
    for (i = 0; i < vol_bound; i++) {
        results[i] = (float)i * 3.14159f;
        sum += results[i];
    }
    
    /* Nested loop with collapse to trigger complex SIMT handling */
    #pragma omp target teams distribute parallel for simd collapse(2) \
            map(tofrom: results[0:m]) map(to: vol_bound) if(vol_bound > 0)
    for (i = 0; i < vol_bound/2; i++) {
        for (j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < m) {
                results[idx] += sum / (idx + 1);
            }
        }
    }
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, float *aux, int size) {
    int i, j;
    int device_ptr_size = size * sizeof(int);
    int *device_ptr = (int*)omp_target_alloc(device_ptr_size, omp_get_default_device());
    
    if (!device_ptr) {
        printf("Device allocation failed\n");
        return;
    }
    
    /* Initialize device data */
    #pragma omp target teams distribute parallel for simd \
            is_device_ptr(device_ptr) map(to: size) if(size > 0)
    for (i = 0; i < size; i++) {
        device_ptr[i] = i * i;
    }
    
    /* Complex nesting: teams with taskloop simd */
    #pragma omp target map(tofrom: data[0:size], aux[0:size/2]) \
            device(simd:1) if(g_volatile_bound == 0)
    {
        #pragma omp teams num_teams(2) thread_limit(64)
        {
            #pragma omp distribute
            for (i = 0; i < 2; i++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd
                    for (j = i * size/2; j < (i + 1) * size/2; j++) {
                        data[j] += device_ptr[j % size];
                        aux[j/2] = (float)data[j] / (j + 1);
                    }
                }
            }
        }
    }
    
    /* Additional SIMD loop with runtime bounds */
    volatile int dyn_bound = size - getpid() % 16;
    #pragma omp target teams distribute simd \
            map(tofrom: data[0:size]) map(to: dyn_bound) \
            if(dyn_bound > BLOCK) device(ancestor:1)
    for (i = 0; i < dyn_bound; i++) {
        data[i] = (data[i] * 13) % 997;
    }
    
    omp_target_free(device_ptr, omp_get_default_device());
}

/* Helper function with mixed constructs */
void test_mixed_constructs(double *matrix, int rows, int cols) {
    int i, j;
    
    #pragma omp target teams distribute parallel for simd collapse(2) \
            map(tofrom: matrix[0:rows*cols]) map(to: rows, cols) \
            device(simd:1) if(rows > 0 && cols > 0)
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            matrix[idx] = (double)(i * cols + j) / (rows * cols);
        }
    }
    
    /* SIMD with linear clause */
    #pragma omp target teams distribute parallel for simd \
            map(tofrom: matrix[0:rows*cols]) \
            linear(i:1) schedule(simd:guided)
    for (i = 0; i < rows * cols; i += 2) {
        matrix[i] = matrix[i] * matrix[i] + 1.0;
    }
}

int main(int argc, char *argv[]) {
    int arr[SIZE];
    float results[SIZE/2];
    double matrix[BLOCK * BLOCK];
    int i, j;
    
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    g_volatile_bound = (argc > 1) ? atoi(argv[1]) : 64;
    
    printf("Starting SIMT transformation tests (PID: %d)\n", getpid());
    printf("Volatile bound: %d\n", g_volatile_bound);
    
    /* Test 1: Basic SIMD transformation */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    #pragma omp parallel for simd
    for (i = 0; i < SIZE; i++) {
        arr[i] = 0;
    }
    
    test_target_teams_distribute_parallel_for_simd(arr, SIZE);
    
    int sum1 = 0;
    #pragma omp simd reduction(+:sum1)
    for (i = 0; i < BLOCK; i++) {
        sum1 += arr[i];
    }
    printf("Checksum 1: %d (global: %d)\n", sum1, g_checksum);
    
    /* Test 2: Teams distribute simd with reduction */
    printf("\nTest 2: target teams distribute simd with reduction\n");
    for (i = 0; i < SIZE/2; i++) {
        results[i] = 0.0f;
    }
    
    test_target_teams_distribute_simd(results, SIZE/2);
    
    float sum2 = 0.0f;
    #pragma omp simd reduction(+:sum2)
    for (i = 0; i < BLOCK/2; i++) {
        sum2 += results[i];
    }
    printf("Checksum 2: %.2f\n", sum2);
    
    /* Test 3: Complex nesting with device pointers */
    printf("\nTest 3: Complex nesting with device pointers\n");
    int data[SIZE];
    float aux[SIZE/2];
    
    #pragma omp parallel for simd
    for (i = 0; i < SIZE; i++) {
        data[i] = i;
        if (i < SIZE/2) aux[i] = 0.0f;
    }
    
    test_complex_nesting(data, aux, SIZE);
    
    int sum3 = 0;
    #pragma omp simd reduction(+:sum3)
    for (i = 0; i < BLOCK; i++) {
        sum3 += data[i];
    }
    printf("Checksum 3: %d\n", sum3);
    
    /* Test 4: Mixed constructs with collapse */
    printf("\nTest 4: Mixed constructs with collapse\n");
    for (i = 0; i < BLOCK * BLOCK; i++) {
        matrix[i] = 0.0;
    }
    
    test_mixed_constructs(matrix, BLOCK, BLOCK);
    
    double sum4 = 0.0;
    #pragma omp simd reduction(+:sum4)
    for (i = 0; i < BLOCK * 2; i++) {
        sum4 += matrix[i];
    }
    printf("Checksum 4: %.4f\n", sum4);
    
    /* Final verification */
    printf("\nAll tests completed.\n");
    printf("Total global checksum: %d\n", g_checksum);
    
    return 0;
}
