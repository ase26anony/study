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
    
    /* Use volatile to force runtime evaluation */
    volatile int bound = n;
    
    /* Complex target region with if clause and device clause */
    #pragma omp target if(bound > 512) device(simd:1) map(tofrom: arr[0:n]) map(from: local_sum) \
                       num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                reduction(+:local_sum) collapse(2)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < BLOCK; j++) {
            int idx = i * BLOCK + j;
            if (idx < n) {
                arr[idx] = arr[idx] * 2 + i - j;
                local_sum += arr[idx];
            }
        }
    }
    
    *result = local_sum;
    g_checksum += local_sum;
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *data, int n, float *sum) {
    int i;
    float local_sum = 0.0f;
    
    /* Use runtime value for condition */
    int pid_mod = getpid() % 100;
    
    /* Device pointer allocation to force complex data environment */
    float *device_ptr = (float *)omp_target_alloc(n * sizeof(float), 0);
    
    if (device_ptr) {
        /* Target with ancestor device clause */
        #pragma omp target device(ancestor:1) if(pid_mod > 50) \
                         map(to: data[0:n]) map(from: local_sum) \
                         is_device_ptr(device_ptr)
        #pragma omp teams distribute simd dist_schedule(static, 16) \
                    reduction(+:local_sum)
        for (i = 0; i < n; i++) {
            float val = data[i];
            device_ptr[i] = val * 3.14159f;
            local_sum += device_ptr[i] * (i % 8);
        }
        
        omp_target_free(device_ptr, 0);
    }
    
    *sum = local_sum;
    g_checksum += (int)local_sum;
}

/* Function 3: Nested target with teams and taskloop simd */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *total) {
    int i, j;
    int grand_total = 0;
    
    /* Volatile bounds to prevent compile-time optimization */
    volatile int v_rows = rows;
    volatile int v_cols = cols;
    
    /* Complex nesting: target -> teams -> distribute -> taskloop simd */
    #pragma omp target if(v_rows * v_cols > 1000) map(tofrom: matrix[0:rows*cols]) \
                       map(from: grand_total) num_teams(4)
    {
        #pragma omp teams reduction(+:grand_total)
        {
            #pragma omp distribute
            for (i = 0; i < v_rows; i++) {
                #pragma omp taskloop simd reduction(+:grand_total) \
                            num_tasks(omp_get_num_threads() * 2)
                for (j = 0; j < v_cols; j++) {
                    int idx = i * cols + j;
                    matrix[idx] = (matrix[idx] + i * 7 - j * 3) % 256;
                    grand_total += matrix[idx];
                }
            }
        }
    }
    
    *total = grand_total;
    g_checksum += grand_total;
}

/* Additional test with collapse and nowait */
void test_simt_wrapper_4(double *a, double *b, double *c, int n) {
    int i, j;
    double checksum = 0.0;
    
    /* Mixed clauses: if, device, nowait */
    #pragma omp target if(n > 256) device(simd:2) \
                       map(to: a[0:n*n], b[0:n*n]) map(from: c[0:n*n]) \
                       nowait depend(out: c)
    #pragma omp teams distribute parallel for simd collapse(2) \
                schedule(static, 8) num_teams(16)
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int idx = i * n + j;
            c[idx] = a[idx] * 2.5 + b[idx] * 1.5;
            #pragma omp atomic
            checksum += c[idx];
        }
    }
    
    #pragma omp taskwait
    
    g_checksum += (int)checksum;
}

int main(int argc, char **argv) {
    int i, j;
    
    /* Initialize with runtime-dependent values */
    int base_size = (argc > 1) ? atoi(argv[1]) : 128;
    if (base_size <= 0) base_size = 128;
    
    /* Test arrays */
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    float *arr2 = (float *)malloc(SIZE * sizeof(float));
    int *matrix = (int *)malloc(SIZE * SIZE / 4 * sizeof(int));
    double *a = (double *)malloc(256 * 256 * sizeof(double));
    double *b = (double *)malloc(256 * 256 * sizeof(double));
    double *c = (double *)malloc(256 * 256 * sizeof(double));
    
    /* Initialize data */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i % 100;
        arr2[i] = (float)i * 0.5f;
    }
    
    for (i = 0; i < SIZE * SIZE / 4; i++) {
        matrix[i] = (i * 3) % 255;
    }
    
    for (i = 0; i < 256 * 256; i++) {
        a[i] = (double)(i % 100) * 0.1;
        b[i] = (double)(i % 50) * 0.2;
    }
    
    /* Set volatile bound based on runtime */
    g_volatile_bound = base_size + (getpid() % 64);
    
    /* Call test functions */
    int result1 = 0;
    float result2 = 0.0f;
    int result3 = 0;
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1 */
    test_simt_wrapper_1(arr1, g_volatile_bound, &result1);
    printf("Test 1 result: %d\n", result1);
    
    /* Test 2 */
    test_simt_wrapper_2(arr2, SIZE, &result2);
    printf("Test 2 result: %f\n", result2);
    
    /* Test 3 */
    test_simt_wrapper_3(matrix, 32, 32, &result3);
    printf("Test 3 result: %d\n", result3);
    
    /* Test 4 */
    test_simt_wrapper_4(a, b, c, 256);
    
    /* Verify results aren't zero */
    int final_check = (result1 != 0) && ((int)result2 != 0) && (result3 != 0);
    
    printf("Global checksum: %d\n", g_checksum);
    printf("Final check: %s\n", final_check ? "PASS" : "FAIL");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(matrix);
    free(a);
    free(b);
    free(c);
    
    return final_check ? 0 : 1;
}
