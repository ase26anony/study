/* Test program to trigger SIMT transformation in omp-low.cc */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define N 1024
#define M 512
#define CHUNK_SIZE 16

/* Global variables to prevent optimization */
volatile int g_volatile_bound = 0;
static int g_checksum = 0;

/* Function 1: target teams distribute parallel for simd with schedule(simd:static) */
void test_simt_wrapper_1(int *arr, int n, int *result) {
    int local_sum = 0;
    volatile int vol_bound = n;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(tofrom: local_sum) map(to: arr[0:n]) \
                      device(ancestor:1) if(0) \
                      teams num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd \
                      schedule(simd:static, CHUNK_SIZE) \
                      reduction(+:local_sum) collapse(2)
    for (int i = 0; i < vol_bound; i += 2) {
        for (int j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx < n) {
                local_sum += arr[idx] * (idx % 8);
            }
        }
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *data, int m, float *sum_result) {
    float sum = 0.0f;
    volatile int dynamic_bound = m + (getpid() % 16);
    
    /* Complex device clause combination */
    #pragma omp target map(tofrom: sum) map(to: data[0:m]) \
                      device(simd:1) if(1) \
                      teams dist_schedule(static, 16)
    #pragma omp teams distribute simd \
                      reduction(+:sum) \
                      safelen(8)
    for (int i = 0; i < dynamic_bound; i++) {
        if (i < m) {
            sum += data[i] * (i % 4 + 1);
        }
    }
    
    /* Nested target region with different clause */
    #pragma omp target map(tofrom: sum) device(0) if(0)
    {
        #pragma omp simd reduction(+:sum)
        for (int i = 0; i < m/2; i++) {
            sum += 1.0f;
        }
    }
    
    *sum_result = sum;
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    /* Allocate device memory explicitly */
    int *dev_matrix = (int *)omp_target_alloc(rows * cols * sizeof(int), 
                                             omp_get_default_device());
    
    if (dev_matrix) {
        /* Copy data to device */
        #pragma omp target enter data map(to: matrix[0:rows*cols]) \
                                    device_data(dev_matrix)
        
        /* Complex target region with multiple constructs */
        #pragma omp target map(from: row_sums[0:rows]) \
                          is_device_ptr(dev_matrix) \
                          device(ancestor:2) if(1) \
                          teams num_teams(4)
        {
            #pragma omp distribute
            for (int r = 0; r < v_rows; r++) {
                int row_sum = 0;
                
                #pragma omp parallel
                {
                    #pragma omp taskloop simd \
                                  collapse(2) \
                                  shared(dev_matrix, row_sum) \
                                  grainsize(8)
                    for (int c = 0; c < v_cols; c += 2) {
                        for (int k = 0; k < 2; k++) {
                            int col = c + k;
                            if (col < cols) {
                                row_sum += dev_matrix[r * cols + col];
                            }
                        }
                    }
                }
                
                row_sums[r] = row_sum;
            }
        }
        
        omp_target_free(dev_matrix, omp_get_default_device());
    }
    
    /* Compute total sum */
    int total = 0;
    for (int i = 0; i < rows; i++) {
        total += row_sums[i];
    }
    g_checksum += total;
}

/* Helper function with runtime-dependent bounds */
void test_variable_bounds(int *output, int size) {
    int base = getpid() % 100;
    
    #pragma omp target teams distribute parallel for simd \
                      map(tofrom: output[0:size]) \
                      device(simd:1) if(base > 50) \
                      schedule(static, 32)
    for (volatile int i = 0; i < size; i++) {
        output[i] = (i + base) * (i % 7);
    }
}

int main(int argc, char **argv) {
    /* Initialize data with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    
    int arr[N];
    float farr[M];
    int matrix[64][32];
    int row_sums[64];
    int output[N];
    int result1, result3_total = 0;
    float result2;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % 100;
        output[i] = 0;
    }
    
    for (int i = 0; i < M; i++) {
        farr[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            matrix[i][j] = rand() % 50;
        }
        row_sums[i] = 0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d, Seed: %d\n", getpid(), seed);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr, N, &result1);
    printf("Result 1: %d\n", result1);
    
    /* Test 2: teams distribute simd with reduction */
    printf("\nTest 2: target teams distribute simd\n");
    test_simt_wrapper_2(farr, M, &result2);
    printf("Result 2: %f\n", result2);
    
    /* Test 3: Complex nesting */
    printf("\nTest 3: Nested taskloop simd in teams\n");
    test_simt_wrapper_3(&matrix[0][0], 64, 32, row_sums);
    for (int i = 0; i < 64; i++) {
        result3_total += row_sums[i];
    }
    printf("Result 3 total: %d\n", result3_total);
    
    /* Test 4: Variable bounds */
    printf("\nTest 4: Variable bounds with runtime condition\n");
    test_variable_bounds(output, N);
    
    /* Verify results */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += output[i];
    }
    printf("Output checksum: %d\n", checksum);
    
    printf("\nGlobal checksum: %d\n", g_checksum);
    
    /* Final target region with mixed clauses */
    int final_check = 0;
    #pragma omp target map(tofrom: final_check) \
                      device(ancestor:1) if(1) \
                      teams distribute parallel for simd \
                      schedule(simd:guided)
    for (int i = 0; i < 100; i++) {
        final_check += i * i;
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
