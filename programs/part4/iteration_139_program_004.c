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
    volatile int vol_bound = n;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(to: arr[0:n]) map(from: result[0:BLOCK]) \
                      if(0) device(simd:1) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
                      schedule(simd:static, 16) collapse(2) \
                      reduction(+:local_sum)
    for (i = 0; i < vol_bound; i += BLOCK) {
        for (j = 0; j < BLOCK; j++) {
            int idx = i + j;
            if (idx < n) {
                result[j] = arr[idx] * 2;
                local_sum += result[j];
            }
        }
    }
    
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule and reduction */
void test_simt_wrapper_2(float *a, float *b, float *c, int n) {
    int i;
    float sum = 0.0f;
    volatile int dynamic_bound = n;
    
    /* Use ancestor device clause - might trigger special handling */
    #pragma omp target map(to: a[0:n], b[0:n]) map(from: c[0:n]) \
                      device(ancestor:1) if(1)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
                      reduction(+:sum) num_teams(4)
    for (i = 0; i < dynamic_bound; i++) {
        c[i] = a[i] + b[i];
        sum += c[i];
    }
    
    /* Store to global to prevent dead code elimination */
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *data, int rows, int cols, int *output) {
    int i, j;
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    /* Allocate device memory explicitly to use is_device_ptr */
    size_t size = rows * cols * sizeof(int);
    int *dev_data = (int *)omp_target_alloc(size, omp_get_default_device());
    int *dev_output = (int *)omp_target_alloc(rows * sizeof(int), 
                                              omp_get_default_device());
    
    if (dev_data && dev_output) {
        /* Copy data to device */
        #pragma omp target enter data map(to: data[0:rows*cols])
        
        /* Complex target region with multiple constructs */
        #pragma omp target if(0) device(simd:1) map(from: output[0:rows]) \
                          is_device_ptr(dev_data, dev_output)
        {
            #pragma omp teams num_teams(rows/16) thread_limit(64)
            {
                #pragma omp distribute
                for (i = 0; i < v_rows; i++) {
                    int row_sum = 0;
                    
                    #pragma omp parallel
                    {
                        #pragma omp taskloop simd collapse(2) \
                                      schedule(simd:guided) reduction(+:row_sum)
                        for (j = 0; j < v_cols; j++) {
                            int idx = i * cols + j;
                            row_sum += data[idx] * (i + 1);
                        }
                    }
                    
                    dev_output[i] = row_sum;
                }
            }
            
            /* Additional SIMD loop */
            #pragma omp simd
            for (i = 0; i < rows; i++) {
                output[i] = dev_output[i] / cols;
            }
        }
        
        #pragma omp target exit data map(from: output[0:rows])
        
        omp_target_free(dev_data, omp_get_default_device());
        omp_target_free(dev_output, omp_get_default_device());
    }
}

/* Helper function with runtime-dependent bounds */
void test_runtime_bounds(int seed) {
    int i;
    int bound = 256 + (seed % 128);  /* Runtime-dependent */
    int arr[512];
    int result[512] = {0};
    
    for (i = 0; i < 512; i++) {
        arr[i] = i * seed;
    }
    
    /* This should trigger the SIMT wrapper generation */
    #pragma omp target map(to: arr[0:512]) map(from: result[0:bound]) \
                      if(seed % 2) device(simd:1)
    #pragma omp teams distribute parallel for simd \
                      schedule(simd:static) num_teams(bound/32)
    for (i = 0; i < bound; i++) {
        result[i] = arr[i] * 3;
    }
    
    /* Compute checksum */
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < bound; i++) {
        sum += result[i];
    }
    g_checksum += sum;
}

int main(int argc, char **argv) {
    int i;
    int pid = getpid();
    
    /* Initialize arrays */
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *result1 = (int *)malloc(BLOCK * sizeof(int));
    float *arr2_f1 = (float *)malloc(SIZE * sizeof(float));
    float *arr2_f2 = (float *)malloc(SIZE * sizeof(float));
    float *arr2_f3 = (float *)malloc(SIZE * sizeof(float));
    int *arr3 = (int *)malloc(SIZE * 8 * sizeof(int));
    int *output3 = (int *)malloc(128 * sizeof(int));
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i + pid;
        arr2_f1[i] = (float)i * 1.5f;
        arr2_f2[i] = (float)i * 2.5f;
    }
    
    for (i = 0; i < SIZE * 8; i++) {
        arr3[i] = (i % 64) + pid;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("PID: %d\n", pid);
    
    /* Test 1: Basic SIMT wrapper */
    printf("\nTest 1: teams distribute parallel for simd\n");
    test_simt_wrapper_1(arr1, SIZE, result1);
    
    /* Verify results */
    int check1 = 0;
    for (i = 0; i < BLOCK; i++) {
        check1 += result1[i];
    }
    printf("Check1 sum: %d\n", check1);
    
    /* Test 2: teams distribute simd with reduction */
    printf("\nTest 2: teams distribute simd with reduction\n");
    test_simt_wrapper_2(arr2_f1, arr2_f2, arr2_f3, SIZE);
    
    /* Verify results */
    float check2 = 0.0f;
    for (i = 0; i < SIZE; i++) {
        check2 += arr2_f3[i];
    }
    printf("Check2 sum: %.2f\n", check2);
    
    /* Test 3: Complex nesting with device pointers */
    printf("\nTest 3: Complex nesting with taskloop simd\n");
    test_simt_wrapper_3(arr3, 128, 64, output3);
    
    /* Verify results */
    int check3 = 0;
    for (i = 0; i < 128; i++) {
        check3 += output3[i];
    }
    printf("Check3 sum: %d\n", check3);
    
    /* Test 4: Runtime bounds */
    printf("\nTest 4: Runtime-dependent bounds\n");
    test_runtime_bounds(pid);
    
    printf("\nFinal global checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(result1);
    free(arr2_f1);
    free(arr2_f2);
    free(arr2_f3);
    free(arr3);
    free(output3);
    
    return 0;
}
