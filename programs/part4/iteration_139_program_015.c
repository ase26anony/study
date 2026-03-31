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
    int pid = getpid() % 100;
    
    #pragma omp target map(tofrom: arr[0:n]) if(pid > 50) device(simd:1)
    #pragma omp teams num_teams(4) thread_limit(64)
    #pragma omp distribute parallel for simd schedule(simd:static, 32)
    for (i = 0; i < n; i++) {
        arr[i] = arr[i] * 2 + pid;
    }
}

/* Function 2: target teams distribute simd with reduction */
void test_target_teams_distribute_simd_reduction(float *results, int n) {
    int i, j;
    float sum = 0.0f;
    volatile int bound = n / 2 + g_volatile_bound;
    
    #pragma omp target map(to: results[0:n]) map(from: sum) \
                     device(ancestor:1) if(0)
    #pragma omp teams num_teams(8) dist_schedule(static, 16)
    #pragma omp distribute simd reduction(+:sum) collapse(2)
    for (i = 0; i < bound; i++) {
        for (j = 0; j < 2; j++) {
            int idx = i * 2 + j;
            if (idx < n) {
                sum += results[idx] * 0.5f;
            }
        }
    }
    
    /* Store result to prevent dead code elimination */
    results[0] = sum;
    g_checksum += (int)sum;
}

/* Function 3: Complex nesting with taskloop simd inside teams */
void test_complex_nesting(int *data, int *dev_ptr, int n) {
    int i;
    
    /* Allocate device memory */
    void *dptr = omp_target_alloc(n * sizeof(int), 0);
    if (!dptr) return;
    
    /* Initialize device data */
    #pragma omp target is_device_ptr(dptr) map(to: data[0:n])
    #pragma omp teams
    {
        #pragma omp distribute simd
        for (i = 0; i < n; i++) {
            ((int*)dptr)[i] = data[i];
        }
        
        #pragma omp parallel
        #pragma omp single
        #pragma omp taskloop simd
        for (i = 0; i < n; i += 2) {
            if (i + 1 < n) {
                int temp = ((int*)dptr)[i];
                ((int*)dptr)[i] = ((int*)dptr)[i + 1];
                ((int*)dptr)[i + 1] = temp;
            }
        }
    }
    
    /* Copy back and free */
    #pragma omp target is_device_ptr(dptr) map(from: data[0:n])
    #pragma omp teams distribute parallel for simd
    for (i = 0; i < n; i++) {
        data[i] = ((int*)dptr)[i];
    }
    
    omp_target_free(dptr, 0);
}

/* Function 4: Additional test with runtime bounds and collapse */
void test_runtime_collapse(double *matrix, int rows, int cols) {
    int i, j;
    volatile int vrows = rows;
    volatile int vcols = cols;
    
    #pragma omp target map(tofrom: matrix[0:rows*cols]) \
                     if(getpid() % 3 == 0) device(simd:2)
    #pragma omp teams distribute parallel for simd collapse(2) \
                     schedule(static, 8) num_teams(rows/16)
    for (i = 0; i < vrows; i++) {
        for (j = 0; j < vcols; j++) {
            int idx = i * cols + j;
            matrix[idx] = matrix[idx] * (i + 1) / (j + 1);
        }
    }
}

int main(int argc, char **argv) {
    int i;
    int arr[SIZE];
    float results[SIZE/2];
    double matrix[BLOCK * BLOCK];
    
    /* Initialize with runtime-dependent values */
    int seed = getpid();
    srand(seed);
    g_volatile_bound = argc > 1 ? atoi(argv[1]) % 10 : 5;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        arr[i] = rand() % 100;
    }
    
    for (i = 0; i < SIZE/2; i++) {
        results[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    for (i = 0; i < BLOCK * BLOCK; i++) {
        matrix[i] = (double)(rand() % 100) / 3.0;
    }
    
    printf("Starting SIMT transformation tests...\n");
    
    /* Test 1: Basic SIMT transformation */
    printf("Test 1: target teams distribute parallel for simd\n");
    test_target_teams_distribute_parallel_for_simd(arr, SIZE);
    
    /* Verify results */
    int sum1 = 0;
    for (i = 0; i < SIZE; i++) {
        sum1 += arr[i];
    }
    printf("  Checksum 1: %d\n", sum1);
    
    /* Test 2: SIMD with reduction */
    printf("Test 2: target teams distribute simd with reduction\n");
    test_target_teams_distribute_simd_reduction(results, SIZE/2);
    printf("  Global checksum: %d\n", g_checksum);
    
    /* Test 3: Complex nesting with device pointers */
    printf("Test 3: Complex nesting with taskloop simd\n");
    int data_copy[SIZE];
    for (i = 0; i < SIZE; i++) {
        data_copy[i] = arr[i];
    }
    test_complex_nesting(data_copy, NULL, SIZE);
    
    /* Verify permutation */
    int diff_count = 0;
    for (i = 0; i < SIZE; i += 2) {
        if (i + 1 < SIZE && data_copy[i] == arr[i + 1]) {
            diff_count++;
        }
    }
    printf("  Permutation pairs: %d\n", diff_count);
    
    /* Test 4: Runtime collapse */
    printf("Test 4: Runtime collapse with 2D matrix\n");
    test_runtime_collapse(matrix, BLOCK, BLOCK);
    
    /* Final verification */
    double final_sum = 0.0;
    for (i = 0; i < BLOCK * BLOCK; i++) {
        final_sum += matrix[i];
    }
    printf("  Matrix sum: %.2f\n", final_sum);
    
    printf("All tests completed.\n");
    
    return 0;
}
