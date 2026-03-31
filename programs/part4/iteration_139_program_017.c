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
void test_target_teams_distribute_parallel_for_simd(int *arr, int n) {
    int i;
    volatile int local_bound = n;
    
    #pragma omp target map(tofrom: arr[0:n]) if(0) device(simd:1) \
        num_teams(8) thread_limit(128)
    #pragma omp teams distribute parallel for simd \
        schedule(simd:static, 16) private(i)
    for (i = 0; i < local_bound; i++) {
        arr[i] = i * 2 + (i % 3);
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd_reduction(float *results, int n) {
    int i, j;
    float sum = 0.0f;
    volatile int chunk = 32;
    
    #pragma omp target map(to: results[0:n]) map(from: sum) \
        device(ancestor:1) if(1)
    #pragma omp teams distribute simd dist_schedule(static, 16) \
        reduction(+:sum) collapse(2)
    for (i = 0; i < n; i += chunk) {
        for (j = 0; j < chunk && (i + j) < n; j++) {
            float val = results[i + j];
            sum += val * val + (i + j);
        }
    }
    
    /* Store result to prevent dead code elimination */
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int *dev_ptr, int n) {
    int i;
    
    /* Allocate device memory */
    void *device_mem = omp_target_alloc(n * sizeof(int), 0);
    if (!device_mem) return;
    
    #pragma omp target is_device_ptr(device_mem) map(tofrom: data[0:n]) \
        if(omp_get_num_devices() > 0)
    {
        int *dev_data = (int *)device_mem;
        
        #pragma omp teams num_teams(4) thread_limit(64)
        {
            #pragma omp distribute
            for (i = 0; i < n; i += BLOCK) {
                #pragma omp taskloop simd num_tasks(16) \
                    shared(dev_data, data) private(i)
                for (int j = i; j < i + BLOCK && j < n; j++) {
                    dev_data[j] = data[j] * 3;
                    data[j] = dev_data[j] + j;
                }
            }
        }
    }
    
    omp_target_free(device_mem, 0);
}

/* Function 4: Mixed constructs with runtime bounds */
void test_mixed_constructs(double *matrix, int rows, int cols) {
    int i, j;
    volatile int vrows = rows;
    volatile int vcols = cols;
    
    #pragma omp target map(tofrom: matrix[0:rows*cols]) \
        if(omp_in_parallel()) device(simd:2)
    #pragma omp teams distribute parallel for simd \
        schedule(static, 8) collapse(2)
    for (i = 0; i < vrows; i++) {
        for (j = 0; j < vcols; j++) {
            int idx = i * cols + j;
            matrix[idx] = (i * 1.5 + j * 0.5) / (idx + 1);
        }
    }
}

/* Helper function to verify results */
int verify_results(int *arr, int n, int expected_sum) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return (sum == expected_sum);
}

int main(int argc, char **argv) {
    int i;
    int *array1 = (int *)malloc(SIZE * sizeof(int));
    float *array2 = (float *)malloc(SIZE * sizeof(float));
    double *matrix = (double *)malloc(SIZE * SIZE * sizeof(double));
    
    if (!array1 || !array2 || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with runtime-dependent values */
    int seed = getpid() + (argc > 1 ? atoi(argv[1]) : 0);
    srand(seed);
    
    for (i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = (float)(rand() % 100) / 10.0f;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = (double)(rand() % 100) / 20.0;
    }
    
    printf("Starting OpenMP SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMD transformation */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(array1, SIZE);
    
    /* Verify Test 1 */
    int expected_sum1 = 0;
    for (i = 0; i < SIZE; i++) {
        expected_sum1 += i * 2 + (i % 3);
    }
    
    if (verify_results(array1, SIZE, expected_sum1)) {
        printf("  Test 1 PASSED\n");
    } else {
        printf("  Test 1 FAILED\n");
    }
    
    /* Test 2: SIMD with reduction */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd_reduction(array2, SIZE);
    printf("  Checksum: %d\n", g_checksum);
    
    /* Test 3: Complex nesting */
    printf("Test 3: Complex nesting with taskloop simd\n");
    test_complex_nesting(array1, NULL, SIZE);
    
    /* Test 4: Mixed constructs */
    printf("Test 4: Mixed constructs with collapse\n");
    test_mixed_constructs(matrix, SIZE, SIZE);
    
    /* Final verification */
    double matrix_sum = 0.0;
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix_sum += matrix[i];
    }
    printf("Matrix sum: %f\n", matrix_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(matrix);
    
    printf("All tests completed.\n");
    return 0;
}
