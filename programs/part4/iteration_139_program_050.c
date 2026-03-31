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
void test_simt_wrapper_1(int *arr, int n, int *result) {
    int i, j;
    int local_sum = 0;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(tofrom: arr[0:n]) map(from: local_sum) \
                      device(simd:1) if(0) num_teams(8)
    #pragma omp teams distribute parallel for simd \
                      schedule(simd:static, 32) reduction(+:local_sum)
    for (i = 0; i < n; i++) {
        /* Complex computation to prevent dead code elimination */
        arr[i] = (arr[i] * 3 + 7) % 256;
        local_sum += arr[i];
        
        /* Nested loop simulation with collapse */
        #pragma omp simd
        for (j = 0; j < 4; j++) {
            arr[i] += j;
        }
    }
    
    *result = local_sum;
    g_volatile_counter++;
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *data, int n, float *sum_result) {
    int i;
    float sum = 0.0f;
    volatile int bound = n; /* Volatile to prevent constant propagation */
    
    /* Use ancestor device clause */
    #pragma omp target map(tofrom: data[0:n]) map(from: sum) \
                      device(ancestor:1) if(1)
    #pragma omp teams distribute simd \
                      dist_schedule(static, 16) reduction(+:sum) \
                      num_teams(4) thread_limit(128)
    for (i = 0; i < bound; i++) {
        data[i] = data[i] * 2.5f + 1.0f;
        sum += data[i];
        
        /* Additional computation with SIMD */
        #pragma omp simd
        for (int k = 0; k < 8; k++) {
            data[i] += k * 0.1f;
        }
    }
    
    *sum_result = sum;
    g_volatile_counter += 2;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *total) {
    int i, j;
    int local_total = 0;
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(rows * cols * sizeof(int), 
                                          omp_get_default_device());
    
    if (dev_ptr) {
        /* Copy data to device */
        #pragma omp target enter data map(to: matrix[0:rows*cols])
        
        /* Target region with is_device_ptr */
        #pragma omp target if(0) map(from: local_total) \
                          is_device_ptr(dev_ptr) device(simd:2)
        {
            #pragma omp teams num_teams(2)
            {
                #pragma omp distribute
                for (i = 0; i < v_rows; i++) {
                    #pragma omp taskloop simd collapse(2) \
                                  grainsize(4) reduction(+:local_total)
                    for (j = 0; j < v_cols; j++) {
                        int idx = i * cols + j;
                        dev_ptr[idx] = matrix[idx] * (i + 1) + (j * 2);
                        local_total += dev_ptr[idx];
                        
                        /* Additional SIMD computation */
                        #pragma omp simd
                        for (int k = 0; k < 2; k++) {
                            dev_ptr[idx] += k * 3;
                        }
                    }
                }
            }
        }
        
        /* Copy results back */
        #pragma omp target exit data map(from: matrix[0:rows*cols])
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    *total = local_total;
    g_volatile_counter += 3;
}

/* Function 4: Mixed constructs with runtime bounds */
void test_simt_wrapper_4(double *a, double *b, int n, double *dot_result) {
    int i;
    double dot = 0.0;
    int pid = getpid(); /* Runtime value for bounds */
    int dynamic_bound = n + (pid % 16); /* Prevent compile-time optimization */
    
    /* Multiple map clauses */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: dot) \
                      if(pid % 2) device(simd:3)
    #pragma omp teams distribute parallel for simd \
                      schedule(static, 8) reduction(+:dot) \
                      collapse(2)
    for (i = 0; i < dynamic_bound; i++) {
        for (int j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < n) {
                a[idx] = a[idx] * 1.5;
                b[idx] = b[idx] * 2.0;
                dot += a[idx] * b[idx];
            }
        }
    }
    
    *dot_result = dot;
    g_volatile_counter += 4;
}

int main(int argc, char *argv[]) {
    int i;
    int test_size = SIZE;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = SIZE;
    }
    
    /* Allocate test arrays */
    int *arr1 = (int *)malloc(test_size * sizeof(int));
    float *arr2 = (float *)malloc(test_size * sizeof(float));
    int *matrix = (int *)malloc(test_size * test_size * sizeof(int));
    double *vec_a = (double *)malloc(test_size * sizeof(double));
    double *vec_b = (double *)malloc(test_size * sizeof(double));
    
    /* Initialize arrays with pattern */
    for (i = 0; i < test_size; i++) {
        arr1[i] = i % 100;
        arr2[i] = (float)(i % 50) * 0.5f;
        vec_a[i] = (double)(i % 25) * 0.25;
        vec_b[i] = (double)(i % 30) * 0.33;
    }
    
    for (i = 0; i < test_size * test_size; i++) {
        matrix[i] = i % 200;
    }
    
    /* Results storage */
    int result1 = 0;
    float result2 = 0.0f;
    int result3 = 0;
    double result4 = 0.0;
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, test_size, &result1);
    printf("  Result1 checksum: %d\n", result1);
    g_checksum += result1;
    
    /* Test 2: teams distribute simd */
    printf("Test 2: target teams distribute simd\n");
    test_simt_wrapper_2(arr2, test_size, &result2);
    printf("  Result2 checksum: %.2f\n", result2);
    g_checksum += (int)result2;
    
    /* Test 3: Complex nesting with taskloop */
    printf("Test 3: Nested teams with taskloop simd\n");
    test_simt_wrapper_3(matrix, test_size, test_size, &result3);
    printf("  Result3 checksum: %d\n", result3);
    g_checksum += result3;
    
    /* Test 4: Mixed constructs with collapse */
    printf("Test 4: Mixed constructs with collapse\n");
    test_simt_wrapper_4(vec_a, vec_b, test_size, &result4);
    printf("  Result4 checksum: %.2f\n", result4);
    g_checksum += (int)result4;
    
    /* Final verification */
    printf("\nFinal volatile counter: %d\n", g_volatile_counter);
    printf("Total checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(vec_a);
    free(vec_b);
    
    return 0;
}
