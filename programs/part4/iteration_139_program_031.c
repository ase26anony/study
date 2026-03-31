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
    
    /* Complex target region with if clause and device clause */
    #pragma omp target if(0) device(simd:1) map(to: arr[0:n]) map(from: result[0:n]) \
                       map(tofrom: local_sum) num_teams(8) thread_limit(64)
    #pragma omp teams distribute parallel for simd schedule(simd:static, 32) \
                       reduction(+:local_sum) collapse(2)
    for (i = 0; i < vol_n; i += BLOCK) {
        for (j = 0; j < BLOCK && (i + j) < n; j++) {
            int idx = i + j;
            result[idx] = arr[idx] * 2 + omp_get_thread_num();
            local_sum += result[idx];
        }
    }
    
    g_checksum += local_sum;
    printf("Test 1 checksum increment: %d\n", local_sum);
}

/* Function 2: target teams distribute simd with dist_schedule */
void test_simt_wrapper_2(float *a, float *b, float *c, int n, float *sum) {
    int i;
    float local_sum = 0.0f;
    
    /* Use runtime value for bounds */
    int bound = n + (getpid() % 16);
    
    /* Device pointer allocation and mapping */
    float *dev_ptr = (float *)omp_target_alloc(n * sizeof(float), omp_get_default_device());
    
    if (dev_ptr) {
        #pragma omp target device(ancestor:1) map(to: a[0:n], b[0:n]) \
                           map(from: c[0:n]) map(tofrom: local_sum) \
                           is_device_ptr(dev_ptr)
        #pragma omp teams distribute simd dist_schedule(static, 16) \
                           reduction(+:local_sum)
        for (i = 0; i < bound; i++) {
            if (i < n) {
                c[i] = a[i] + b[i] * 2.0f;
                dev_ptr[i] = c[i] * 0.5f;  /* Use device pointer */
                local_sum += c[i];
            }
        }
        
        omp_target_free(dev_ptr, omp_get_default_device());
    }
    
    *sum = local_sum;
    printf("Test 2 sum: %f\n", local_sum);
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_simt_wrapper_3(int *matrix, int rows, int cols, int *row_sums) {
    int i, j;
    
    /* Nested loops with collapse */
    #pragma omp target if(rows > 100) map(to: matrix[0:rows*cols]) \
                       map(from: row_sums[0:rows]) device(simd:1)
    #pragma omp teams distribute
    for (i = 0; i < rows; i++) {
        #pragma omp parallel
        {
            #pragma omp taskloop simd collapse(2) shared(matrix, row_sums) \
                               private(j) grainsize(4)
            for (i = 0; i < rows; i++) {
                for (j = 0; j < cols; j++) {
                    int idx = i * cols + j;
                    row_sums[i] += matrix[idx] * (i + 1);
                }
            }
        }
    }
    
    /* Compute total sum */
    int total = 0;
    #pragma omp simd reduction(+:total)
    for (i = 0; i < rows; i++) {
        total += row_sums[i];
    }
    
    g_checksum += total;
    printf("Test 3 total: %d\n", total);
}

/* Additional test with runtime-dependent SIMT condition */
void test_simt_conditional(int *data, int n) {
    int i;
    int result = 0;
    
    /* Make bound runtime-dependent */
    int dynamic_bound = n + (getpid() & 0xF);
    
    /* Use both if clause and device clause */
    #pragma omp target if(dynamic_bound > 512) device(simd:1) \
                       map(to: data[0:n]) map(tofrom: result)
    #pragma omp teams distribute parallel for simd \
                       schedule(static, 8) reduction(+:result)
    for (i = 0; i < dynamic_bound; i++) {
        if (i < n) {
            data[i] = data[i] * 3 - i;
            result += data[i];
        }
    }
    
    printf("Test 4 result: %d\n", result);
}

int main(int argc, char **argv) {
    int i;
    
    /* Initialize data arrays */
    int *arr1 = (int *)malloc(SIZE * sizeof(int));
    int *res1 = (int *)malloc(SIZE * sizeof(int));
    float *arr2_a = (float *)malloc(SIZE * sizeof(float));
    float *arr2_b = (float *)malloc(SIZE * sizeof(float));
    float *arr2_c = (float *)malloc(SIZE * sizeof(float));
    int *matrix = (int *)malloc(SIZE * SIZE * sizeof(int));
    int *row_sums = (int *)calloc(SIZE, sizeof(int));
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i % 100;
        arr2_a[i] = i * 0.5f;
        arr2_b[i] = i * 0.25f;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (i % 100) + 1;
    }
    
    /* Set volatile bound from command line if provided */
    if (argc > 1) {
        g_volatile_bound = atoi(argv[1]);
    } else {
        g_volatile_bound = SIZE / 2;
    }
    
    printf("Starting SIMT transformation tests...\n");
    printf("Using volatile bound: %d\n", g_volatile_bound);
    
    /* Test 1: Basic SIMT wrapper */
    test_simt_wrapper_1(arr1, g_volatile_bound, res1);
    
    /* Test 2: With device pointers */
    float sum2;
    test_simt_wrapper_2(arr2_a, arr2_b, arr2_c, SIZE, &sum2);
    
    /* Test 3: Complex nesting */
    test_simt_wrapper_3(matrix, 32, 32, row_sums);
    
    /* Test 4: Conditional SIMT */
    test_simt_conditional(arr1, SIZE);
    
    /* Verify results */
    int final_check = 0;
    for (i = 0; i < SIZE; i++) {
        final_check += res1[i];
        final_check += (int)arr2_c[i];
    }
    
    for (i = 0; i < 32; i++) {
        final_check += row_sums[i];
    }
    
    printf("\nFinal checksum: %d\n", final_check);
    printf("Global checksum: %d\n", g_checksum);
    
    /* Cleanup */
    free(arr1);
    free(res1);
    free(arr2_a);
    free(arr2_b);
    free(arr2_c);
    free(matrix);
    free(row_sums);
    
    return 0;
}
