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
    int local_sum = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int vol_n = n;
    
    /* Complex target region with SIMD schedule */
    #pragma omp target map(to: arr[0:vol_n]) map(from: result[0:vol_n]) \
                      if(0) device(simd:1) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
                schedule(simd:static, 32) collapse(2) \
                reduction(+:local_sum)
    for (i = 0; i < vol_n; i += BLOCK) {
        for (j = 0; j < BLOCK && (i + j) < vol_n; j++) {
            int idx = i + j;
            result[idx] = arr[idx] * 2 + omp_get_team_num();
            local_sum += result[idx];
        }
    }
    
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *data, int n, float *output) {
    int i;
    float sum = 0.0f;
    volatile int bound = n;
    
    /* Device pointer allocation to force complex data environment */
    float *device_ptr = (float *)omp_target_alloc(n * sizeof(float), 
                                                  omp_get_default_device());
    
    if (device_ptr) {
        #pragma omp target is_device_ptr(device_ptr) map(to: data[0:bound]) \
                        map(from: output[0:bound]) device(ancestor:1)
        #pragma omp teams distribute simd dist_schedule(static, 16) \
                    reduction(+:sum)
        for (i = 0; i < bound; i++) {
            device_ptr[i] = data[i] * 3.14f;
            output[i] = device_ptr[i] + (float)omp_get_team_num();
            sum += output[i];
        }
        
        omp_target_free(device_ptr, omp_get_default_device());
    }
    
    g_checksum += (int)sum;
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    int i, j;
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    /* Complex nesting to trigger transformation */
    #pragma omp target map(to: matrix[0:v_rows*v_cols]) \
                      map(from: row_sums[0:v_rows]) \
                      if(omp_get_num_devices() > 0)
    {
        #pragma omp teams num_teams(4) thread_limit(64)
        {
            #pragma omp distribute
            for (i = 0; i < v_rows; i++) {
                int row_sum = 0;
                
                /* Taskloop with SIMD inside teams region */
                #pragma omp taskloop simd reduction(+:row_sum) \
                            grainsize(8) simdlen(4)
                for (j = 0; j < v_cols; j++) {
                    int idx = i * v_cols + j;
                    row_sum += matrix[idx] * (i + 1) * (j + 1);
                }
                
                row_sums[i] = row_sum;
            }
        }
    }
    
    /* Add to global checksum */
    for (i = 0; i < rows; i++) {
        g_checksum += row_sums[i];
    }
}

/* Helper function with runtime-dependent bounds */
void test_dynamic_bounds(int seed) {
    int i;
    int size = 256 + (seed % 128);  /* Runtime-dependent size */
    int *array = (int *)malloc(size * sizeof(int));
    int *result = (int *)malloc(size * sizeof(int));
    
    if (!array || !result) return;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        array[i] = i * seed;
    }
    
    /* Target with collapse and nowait */
    #pragma omp target teams distribute parallel for simd \
                map(to: array[0:size]) map(from: result[0:size]) \
                collapse(2) nowait
    for (i = 0; i < size/2; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < size) {
                result[idx] = array[idx] + omp_get_thread_num();
            }
        }
    }
    
    #pragma omp taskwait
    
    /* Verify results */
    int check = 0;
    for (i = 0; i < size; i++) {
        check += result[i];
    }
    g_checksum += check;
    
    free(array);
    free(result);
}

int main(int argc, char **argv) {
    int i;
    int pid = getpid();
    
    /* Initialize data arrays */
    int arr[SIZE];
    float farr[SIZE];
    int matrix[64][64];
    int results_int[SIZE];
    float results_float[SIZE];
    int row_sums[64];
    
    /* Initialize with patterns */
    for (i = 0; i < SIZE; i++) {
        arr[i] = i * 3 + pid;
        farr[i] = (float)i * 0.5f + (float)pid;
        results_int[i] = 0;
        results_float[i] = 0.0f;
    }
    
    for (i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            matrix[i][j] = i * 64 + j;
        }
        row_sums[i] = 0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d\n", pid);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, SIZE, results_int);
    
    /* Verify results */
    int sum1 = 0;
    for (i = 0; i < SIZE; i++) {
        sum1 += results_int[i];
    }
    printf("  Checksum 1: %d\n", sum1);
    
    /* Test 2: SIMD with device pointers */
    printf("\nTest 2: target teams distribute simd with device pointers\n");
    test_simt_wrapper_2(farr, SIZE, results_float);
    
    /* Verify results */
    float sum2 = 0.0f;
    for (i = 0; i < SIZE; i++) {
        sum2 += results_float[i];
    }
    printf("  Checksum 2: %f\n", sum2);
    
    /* Test 3: Nested teams with taskloop simd */
    printf("\nTest 3: Nested target with teams and taskloop simd\n");
    test_simt_wrapper_3(&matrix[0][0], 64, 64, row_sums);
    
    /* Verify results */
    int sum3 = 0;
    for (i = 0; i < 64; i++) {
        sum3 += row_sums[i];
    }
    printf("  Checksum 3: %d\n", sum3);
    
    /* Test 4: Dynamic bounds */
    printf("\nTest 4: Dynamic bounds with collapse\n");
    test_dynamic_bounds(pid);
    
    printf("\nGlobal checksum: %d\n", g_checksum);
    
    /* Final verification */
    if (g_checksum != 0) {
        printf("\nAll tests completed successfully.\n");
        return 0;
    } else {
        printf("\nError: All checksums zero!\n");
        return 1;
    }
}
