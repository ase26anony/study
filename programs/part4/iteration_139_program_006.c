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
    int local_sum = 0;
    int pid_mod = getpid() % 100;
    
    /* Use device clause that might trigger SIMT path */
    #pragma omp target map(tofrom: local_sum) map(to: arr[0:n]) \
                      device(ancestor:1) if(pid_mod > 50)
    #pragma omp teams distribute parallel for simd \
                      num_teams(4) thread_limit(32) \
                      schedule(simd:static, BLOCK) reduction(+:local_sum)
    for (int i = 0; i < n; i++) {
        local_sum += arr[i] * (i % 16);
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *a, float *b, float *c, int n, float *sum) {
    volatile int vol_n = n + g_volatile_bound;
    float local_sum = 0.0f;
    
    /* Complex device clause combination */
    #pragma omp target map(to: a[0:vol_n], b[0:vol_n]) map(from: c[0:vol_n]) \
                      map(tofrom: local_sum) device(simd:1) if(0)
    #pragma omp teams distribute simd \
                      dist_schedule(static, 16) reduction(+:local_sum) \
                      collapse(2)
    for (int i = 0; i < vol_n; i += 2) {
        for (int j = 0; j < 2; j++) {
            int idx = i + j;
            if (idx < vol_n) {
                c[idx] = a[idx] * b[idx] + (float)(idx % 8);
                local_sum += c[idx];
            }
        }
    }
    
    *sum = local_sum;
    g_checksum += (int)local_sum;
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *data, int rows, int cols, int *total) {
    int pid = getpid();
    volatile int vol_rows = rows + (pid % 3);
    int local_total = 0;
    
    /* Allocate device memory explicitly */
    int *dev_ptr = (int *)omp_target_alloc(vol_rows * cols * sizeof(int), 
                                          omp_get_default_device());
    
    if (dev_ptr) {
        /* Complex nesting with is_device_ptr */
        #pragma omp target map(to: data[0:vol_rows*cols]) \
                          map(from: local_total) \
                          is_device_ptr(dev_ptr) device(ancestor:1)
        {
            #pragma omp teams num_teams(2)
            {
                #pragma omp distribute
                for (int i = 0; i < vol_rows; i++) {
                    #pragma omp parallel
                    {
                        #pragma omp taskloop simd \
                                  simdlen(8) nogroup
                        for (int j = 0; j < cols; j++) {
                            int idx = i * cols + j;
                            dev_ptr[idx] = data[idx] * (i + 1) + (j % 4);
                            #pragma omp atomic
                            local_total += dev_ptr[idx];
                        }
                    }
                }
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    *total = local_total;
    g_checksum += local_total;
}

/* Function 4: Mixed constructs with runtime bounds */
void test_simt_wrapper_4(double *in, double *out, int size, double *result) {
    volatile int dynamic_size = size + (getpid() % 10);
    double local_result = 0.0;
    
    /* Multiple clauses to trigger complex lowering */
    #pragma omp target map(to: in[0:dynamic_size]) map(from: out[0:dynamic_size]) \
                      map(tofrom: local_result) \
                      if(dynamic_size > 100) device(simd:2)
    {
        #pragma omp teams distribute parallel for simd \
                      schedule(static, 32) reduction(+:local_result) \
                      num_teams(8)
        for (int i = 0; i < dynamic_size; i++) {
            out[i] = in[i] * 2.5 + (i % 7);
            local_result += out[i];
        }
        
        /* Additional SIMD loop in same region */
        #pragma omp simd reduction(+:local_result)
        for (int i = 0; i < dynamic_size/2; i++) {
            local_result += out[i] * 0.5;
        }
    }
    
    *result = local_result;
    g_checksum += (int)local_result;
}

int main(int argc, char *argv[]) {
    int arr[SIZE];
    float fa[SIZE], fb[SIZE], fc[SIZE];
    int matrix_data[64][32];
    double din[SIZE], dout[SIZE];
    
    int result1 = 0;
    float result2 = 0.0f;
    int result3 = 0;
    double result4 = 0.0;
    
    /* Initialize with non-trivial patterns */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3) % 97;
        fa[i] = (float)(i % 19) * 0.7f;
        fb[i] = (float)(i % 23) * 1.3f;
        din[i] = (double)(i % 31) * 0.3;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 32; j++) {
            matrix_data[i][j] = i * 32 + j;
        }
    }
    
    /* Set volatile bound based on command line */
    if (argc > 1) {
        g_volatile_bound = atoi(argv[1]) % 100;
    } else {
        g_volatile_bound = getpid() % 50;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("Volatile bound: %d\n", g_volatile_bound);
    
    /* Call all test functions */
    test_simt_wrapper_1(arr, SIZE, &result1);
    printf("Test 1 result: %d\n", result1);
    
    test_simt_wrapper_2(fa, fb, fc, SIZE, &result2);
    printf("Test 2 result: %.2f\n", result2);
    
    test_simt_wrapper_3(&matrix_data[0][0], 64, 32, &result3);
    printf("Test 3 result: %d\n", result3);
    
    test_simt_wrapper_4(din, dout, SIZE, &result4);
    printf("Test 4 result: %.2f\n", result4);
    
    printf("Final checksum: %d\n", g_checksum);
    
    /* Verify results are non-zero (basic sanity check) */
    if (result1 == 0 && result2 == 0.0f && result3 == 0 && result4 == 0.0) {
        printf("WARNING: All results are zero - possible optimization issue\n");
        return 1;
    }
    
    return 0;
}
