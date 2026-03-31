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
    int i, j;
    int pid = getpid() % 100;
    volatile int bound = n + pid;
    
    #pragma omp target if(0) device(simd:1) map(tofrom: arr[0:n]) map(to: n, bound) map(from: result[0:1])
    #pragma omp teams distribute parallel for simd schedule(simd:static) num_teams(4) thread_limit(128)
    for (i = 0; i < bound; i++) {
        if (i < n) {
            arr[i] = arr[i] * 2 + i;
        }
    }
    
    /* Compute checksum */
    result[0] = 0;
    #pragma omp simd reduction(+:result[0])
    for (i = 0; i < n && i < bound; i++) {
        result[0] += arr[i];
    }
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int m, float *sum_out) {
    int i, j;
    float sum = 0.0f;
    volatile int v_m = m;
    
    /* Allocate device memory explicitly */
    float *dev_data = (float *)omp_target_alloc(v_m * sizeof(float), omp_get_default_device());
    
    if (dev_data) {
        #pragma omp target is_device_ptr(dev_data) device(ancestor:1) \
                map(to: v_m) map(tofrom: sum)
        #pragma omp teams distribute simd dist_schedule(static, 16) reduction(+:sum) \
                num_teams(8)
        for (i = 0; i < v_m; i++) {
            dev_data[i] = (float)i * 1.5f;
            sum += dev_data[i];
        }
        
        /* Copy back if needed */
        #pragma omp target is_device_ptr(dev_data) map(from: data[0:v_m])
        #pragma omp teams distribute parallel for simd
        for (i = 0; i < v_m; i++) {
            data[i] = dev_data[i];
        }
        
        omp_target_free(dev_data, omp_get_default_device());
    }
    
    *sum_out = sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *total) {
    int i, j;
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    int local_total = 0;
    
    #pragma omp target map(to: v_rows, v_cols) map(tofrom: matrix[0:rows*cols], local_total) \
            if(rows > 100) device(simd:2)
    {
        #pragma omp teams num_teams(2) thread_limit(64)
        {
            #pragma omp distribute
            for (i = 0; i < v_rows; i++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd collapse(2) grainsize(8)
                    for (j = 0; j < v_cols; j++) {
                        int idx = i * v_cols + j;
                        matrix[idx] = (i + j) * 3;
                        #pragma omp atomic
                        local_total += matrix[idx];
                    }
                }
            }
        }
    }
    
    *total = local_total;
}

/* Helper function with nested loops and collapse */
void test_collapse_simd(int *arr3d, int x, int y, int z) {
    int i, j, k;
    volatile int vx = x, vy = y, vz = z;
    
    #pragma omp target teams distribute parallel for simd collapse(3) \
            map(tofrom: arr3d[0:x*y*z]) map(to: vx, vy, vz) \
            device(ancestor:2) if(1)
    for (i = 0; i < vx; i++) {
        for (j = 0; j < vy; j++) {
            for (k = 0; k < vz; k++) {
                int idx = (i * vy + j) * vz + k;
                arr3d[idx] = (i * 100 + j * 10 + k) * 2;
            }
        }
    }
}

int main(int argc, char **argv) {
    int i;
    int arr1[SIZE];
    float arr2[SIZE];
    int matrix[16][16];
    int arr3d[8][8][8];
    int result1 = 0;
    float result2 = 0.0f;
    int result3 = 0;
    int result4 = 0;
    
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    
    for (i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = (float)(rand() % 100) / 10.0f;
    }
    
    /* Call test functions with different patterns */
    printf("Test 1: teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, SIZE, &result1);
    printf("Checksum 1: %d\n", result1);
    
    printf("\nTest 2: teams distribute simd with reduction\n");
    test_simt_wrapper_2(arr2, SIZE, &result2);
    printf("Checksum 2: %.2f\n", result2);
    
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_simt_wrapper_3(&matrix[0][0], 16, 16, &result3);
    printf("Checksum 3: %d\n", result3);
    
    printf("\nTest 4: Collapse(3) with SIMD\n");
    test_collapse_simd(&arr3d[0][0][0], 8, 8, 8);
    
    /* Verify arr3d results */
    for (i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                result4 += arr3d[i][j][k];
            }
        }
    }
    printf("Checksum 4: %d\n", result4);
    
    /* Final verification */
    int final_check = (result1 > 0) + (result2 > 0.0f) + (result3 > 0) + (result4 > 0);
    printf("\nFinal: %d/4 tests passed verification\n", final_check);
    
    return (final_check == 4) ? 0 : 1;
}
