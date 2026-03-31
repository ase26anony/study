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
    int i;
    int local_sum = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int vol_n = n;
    
    /* Target region with if clause and device clause to trigger SIMT path */
    #pragma omp target if(0) device(simd:1) map(to: arr[0:vol_n]) map(from: result[0:vol_n]) \
                       map(tofrom: local_sum) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                       reduction(+:local_sum)
    for (i = 0; i < vol_n; i++) {
        result[i] = arr[i] * 2 + i;
        local_sum += result[i];
    }
    
    /* Store result to prevent dead code elimination */
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *a, float *b, float *c, int n) {
    int i;
    float sum = 0.0f;
    
    /* Use runtime value for bound */
    int bound = n + getpid() % 16;
    
    /* Target region with ancestor device clause */
    #pragma omp target device(ancestor:1) map(to: a[0:bound], b[0:bound]) \
                       map(from: c[0:bound]) map(tofrom: sum)
    #pragma omp teams distribute simd dist_schedule(static, 16) reduction(+:sum)
    for (i = 0; i < bound; i++) {
        c[i] = a[i] + b[i] * 2.0f;
        sum += c[i];
    }
    
    /* Nested loop with collapse to trigger complex SIMT handling */
    #pragma omp target teams distribute parallel for simd collapse(2) \
                       map(to: a[0:n], b[0:n]) map(from: c[0:n])
    for (i = 0; i < n/BLOCK; i++) {
        for (int j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            c[idx] = a[idx] - b[idx];
        }
    }
    
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *data, int rows, int cols, int *output) {
    int i, j;
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(rows * cols * sizeof(int), 
                                          omp_get_default_device());
    
    if (dev_ptr) {
        /* Target region with is_device_ptr */
        #pragma omp target is_device_ptr(dev_ptr) map(to: data[0:rows*cols]) \
                           map(from: output[0:rows*cols]) if(rows > 0)
        {
            #pragma omp teams distribute
            for (i = 0; i < rows; i++) {
                #pragma omp parallel
                {
                    #pragma omp taskloop simd
                    for (j = 0; j < cols; j++) {
                        int idx = i * cols + j;
                        dev_ptr[idx] = data[idx] * 3;
                        output[idx] = dev_ptr[idx] + idx;
                    }
                }
            }
        }
        
        /* Free device memory */
        omp_target_free(dev_ptr, omp_get_default_device());
    }
}

/* Function 4: Mixed constructs with runtime bounds */
void test_simt_wrapper_4(double *in, double *out, int size) {
    int i;
    double max_val = 0.0;
    
    /* Volatile bound to prevent optimization */
    volatile int dyn_size = size - (getpid() % 8);
    
    /* Target with teams distribute parallel for simd */
    #pragma omp target map(to: in[0:dyn_size]) map(from: out[0:dyn_size]) \
                       map(tofrom: max_val) if(dyn_size > 32)
    #pragma omp teams distribute parallel for simd reduction(max:max_val) \
                       num_teams(4)
    for (i = 0; i < dyn_size; i++) {
        out[i] = in[i] * 1.5;
        if (out[i] > max_val) max_val = out[i];
    }
    
    /* Second loop with different schedule */
    #pragma omp target teams distribute parallel for simd schedule(static, 8) \
                       map(to: in[0:size]) map(from: out[size/2:size/2])
    for (i = size/2; i < size; i++) {
        out[i] = in[i] / 2.0;
    }
    
    g_checksum += (int)(max_val * 100);
}

int main(int argc, char *argv[]) {
    int i;
    int test_size = SIZE;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        test_size = atoi(argv[1]);
        if (test_size <= 0) test_size = SIZE;
    }
    
    /* Initialize data arrays */
    int *arr1 = (int *)malloc(test_size * sizeof(int));
    int *res1 = (int *)malloc(test_size * sizeof(int));
    
    float *arr2_a = (float *)malloc(test_size * sizeof(float));
    float *arr2_b = (float *)malloc(test_size * sizeof(float));
    float *res2 = (float *)malloc(test_size * sizeof(float));
    
    int *arr3 = (int *)malloc(test_size * test_size/16 * sizeof(int));
    int *res3 = (int *)malloc(test_size * test_size/16 * sizeof(int));
    
    double *arr4 = (double *)malloc(test_size * sizeof(double));
    double *res4 = (double *)malloc(test_size * sizeof(double));
    
    /* Initialize with pattern */
    for (i = 0; i < test_size; i++) {
        arr1[i] = i % 100;
        arr2_a[i] = (float)(i % 50) * 0.5f;
        arr2_b[i] = (float)(i % 30) * 0.3f;
        arr4[i] = (double)(i % 80) * 0.25;
    }
    
    for (i = 0; i < test_size * test_size/16; i++) {
        arr3[i] = i % 200;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, test_size, res1);
    
    /* Verify results */
    int sum1 = 0;
    for (i = 0; i < test_size; i++) {
        sum1 += res1[i];
    }
    printf("  Checksum 1: %d\n", sum1);
    
    /* Test 2: teams distribute simd with reduction */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_simt_wrapper_2(arr2_a, arr2_b, res2, test_size);
    
    /* Verify results */
    float sum2 = 0.0f;
    for (i = 0; i < test_size; i++) {
        sum2 += res2[i];
    }
    printf("  Checksum 2: %.2f\n", sum2);
    
    /* Test 3: Complex nesting with taskloop simd */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_simt_wrapper_3(arr3, test_size/16, test_size/16, res3);
    
    /* Verify results */
    int sum3 = 0;
    for (i = 0; i < test_size * test_size/16; i++) {
        sum3 += res3[i];
    }
    printf("  Checksum 3: %d\n", sum3);
    
    /* Test 4: Mixed constructs */
    printf("Test 4: Mixed constructs with runtime bounds\n");
    test_simt_wrapper_4(arr4, res4, test_size);
    
    /* Verify results */
    double max4 = 0.0;
    for (i = 0; i < test_size; i++) {
        if (res4[i] > max4) max4 = res4[i];
    }
    printf("  Max value 4: %.2f\n", max4);
    
    printf("Global checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(res1);
    free(arr2_a);
    free(arr2_b);
    free(res2);
    free(arr3);
    free(res3);
    free(arr4);
    free(res4);
    
    return 0;
}
