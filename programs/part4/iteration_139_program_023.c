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
    
    /* Complex target region with SIMD schedule */
    #pragma omp target map(tofrom: arr[0:n]) map(from: local_sum) \
                     if(0) device(simd:1) num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
                     schedule(simd:static, 32) reduction(+:local_sum) \
                     collapse(2)
    for (i = 0; i < bound; i += 2) {
        for (j = 0; j < 4; j++) {
            int idx = i + j;
            if (idx < n) {
                arr[idx] = arr[idx] * 2 + (i % 16);
                local_sum += arr[idx];
            }
        }
    }
    
    /* Additional nested SIMD region */
    #pragma omp target teams distribute simd \
                     map(tofrom: arr[0:n]) map(tofrom: local_sum) \
                     device(ancestor:1) dist_schedule(static, 16)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] + local_sum % 256;
    }
    
    *result = local_sum;
}

/* Function 2: target teams distribute simd with reduction and device pointer */
void test_simt_wrapper_2(float *data, int n, float *output) {
    int i;
    float sum = 0.0f;
    float *device_ptr = NULL;
    
    /* Allocate device memory explicitly */
    size_t size = n * sizeof(float);
    device_ptr = (float *)omp_target_alloc(size, omp_get_default_device());
    
    if (device_ptr) {
        /* Copy data to device */
        #pragma omp target enter data map(to: data[0:n]) \
                         device(omp_get_default_device())
        
        /* Use is_device_ptr with SIMD construct */
        #pragma omp target teams distribute simd \
                         is_device_ptr(device_ptr) map(tofrom: sum) \
                         reduction(+:sum) if(1) device(simd:2)
        for (i = 0; i < n; i++) {
            float val = data[i] * 1.5f;
            device_ptr[i] = val;
            sum += val * (i % 8);
        }
        
        /* Nested loop with runtime bounds */
        int dynamic_bound = n - (getpid() % 32);
        #pragma omp target teams distribute parallel for simd \
                         map(from: output[0:n]) \
                         schedule(static, 8) num_teams(4) \
                         device(ancestor:2)
        for (i = 0; i < dynamic_bound; i++) {
            output[i] = device_ptr[i] / (sum + 1.0f);
        }
        
        /* Copy back and free */
        #pragma omp target exit data map(from: data[0:n]) \
                         device(omp_get_default_device())
        omp_target_free(device_ptr, omp_get_default_device());
    }
    
    /* Store result in global to prevent DCE */
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *results) {
    int i, j;
    
    /* Use argv-dependent bounds */
    volatile int vrows = rows + (g_volatile_bound % 16);
    volatile int vcols = cols;
    
    /* Target region with teams and taskloop simd */
    #pragma omp target map(tofrom: matrix[0:rows*cols]) \
                     map(tofrom: results[0:rows]) \
                     if(0) device(simd:3)
    {
        #pragma omp teams num_teams(4) thread_limit(64)
        {
            #pragma omp distribute
            for (i = 0; i < vrows; i++) {
                int row_sum = 0;
                
                /* Taskloop with SIMD inside distributed loop */
                #pragma omp taskloop simd reduction(+:row_sum) \
                                 grainsize(8) num_tasks(16)
                for (j = 0; j < vcols; j++) {
                    int idx = i * cols + j;
                    matrix[idx] = matrix[idx] * 3 - (j % 4);
                    row_sum += matrix[idx];
                }
                
                results[i] = row_sum;
                
                /* Additional SIMD loop */
                #pragma omp simd
                for (j = 0; j < 16 && j < vcols; j++) {
                    int idx = i * cols + j;
                    matrix[idx] += results[i] % 128;
                }
            }
        }
    }
    
    /* Second target region with collapse */
    #pragma omp target teams distribute parallel for simd \
                     map(tofrom: matrix[0:rows*cols]) \
                     collapse(2) schedule(simd:guided) \
                     device(ancestor:3)
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            int idx = i * cols + j;
            matrix[idx] = (matrix[idx] * 2) / (results[i % rows] + 1);
        }
    }
}

/* Helper function with mixed constructs */
void test_mixed_simd(int *a, int *b, int n) {
    int i;
    
    /* Multiple SIMD constructs in sequence */
    #pragma omp target simd map(tofrom: a[0:n]) if(1) device(simd:4)
    for (i = 0; i < n; i++) {
        a[i] = a[i] * 2 + i;
    }
    
    #pragma omp target teams distribute simd \
                 map(to: a[0:n]) map(from: b[0:n]) \
                 dist_schedule(static, 32) device(ancestor:4)
    for (i = 0; i < n; i += 2) {
        b[i] = a[i] + a[i + 1];
        if (i > 0) b[i - 1] = a[i] - a[i - 1];
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float farr[SIZE];
    int matrix[BLOCK][BLOCK];
    int results[SIZE];
    float output[SIZE];
    int checksum1, checksum2, checksum3;
    float fsum;
    
    /* Initialize with non-constant values */
    g_volatile_bound = argc > 1 ? atoi(argv[1]) : 64;
    if (g_volatile_bound <= 0) g_volatile_bound = 64;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + getpid()) % 256;
        farr[i] = (float)(i * 2 + getpid() % 128) / 256.0f;
        results[i] = 0;
        output[i] = 0.0f;
    }
    
    for (i = 0; i < BLOCK * BLOCK; i++) {
        ((int *)matrix)[i] = (i * 7 + getpid()) % 512;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT wrapper */
    test_simt_wrapper_1(arr, SIZE, &checksum1);
    
    /* Verify results */
    int verify1 = 0;
    for (i = 0; i < SIZE; i++) {
        verify1 += arr[i];
    }
    printf("Test 1: checksum1 = %d, verify1 = %d\n", checksum1, verify1);
    
    /* Test 2: Device pointers and reduction */
    test_simt_wrapper_2(farr, SIZE, output);
    
    /* Verify results */
    float verify2 = 0.0f;
    for (i = 0; i < SIZE; i++) {
        verify2 += output[i];
    }
    printf("Test 2: g_checksum = %d, verify2 = %.2f\n", g_checksum, verify2);
    
    /* Test 3: Complex nesting */
    test_simt_wrapper_3((int *)matrix, BLOCK, BLOCK, results);
    
    /* Verify results */
    int verify3 = 0;
    for (i = 0; i < BLOCK; i++) {
        verify3 += results[i];
        for (int j = 0; j < BLOCK; j++) {
            verify3 += matrix[i][j];
        }
    }
    printf("Test 3: verify3 = %d\n", verify3);
    
    /* Test 4: Mixed constructs */
    int mixed_a[256], mixed_b[256];
    for (i = 0; i < 256; i++) {
        mixed_a[i] = i + getpid() % 64;
        mixed_b[i] = 0;
    }
    test_mixed_simd(mixed_a, mixed_b, 256);
    
    int verify4 = 0;
    for (i = 0; i < 256; i++) {
        verify4 += mixed_a[i] + mixed_b[i];
    }
    printf("Test 4: verify4 = %d\n", verify4);
    
    /* Final verification */
    int final_check = checksum1 + g_checksum + verify3 + verify4;
    printf("Final combined checksum: %d\n", final_check);
    
    if (final_check != 0) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Warning: All checksums zero - possible optimization issue.\n");
        return 1;
    }
}
