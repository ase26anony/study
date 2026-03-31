/* Test program to cover all partition code cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 16

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_sum) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return (fabs(sum - expected_sum) < 1e-6);
}

int main() {
    int errors = 0;
    
    /* Test data arrays */
    double *arr1d = (double*)malloc(N * sizeof(double));
    double *arr2d = (double*)malloc(N * M * sizeof(double));
    double *arr3d = (double*)malloc(N * M * P * sizeof(double));
    double *arr_small = (double*)malloc(M * sizeof(double));
    
    if (!arr1d || !arr2d || !arr3d || !arr_small) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_array(arr1d, N, 1.0);
    init_array(arr2d, N * M, 2.0);
    init_array(arr3d, N * M * P, 3.0);
    init_array(arr_small, M, 0.5);
    
    printf("Testing OpenACC partition cases...\n");
    
    /* ============================================
     * CASE 0: gang redundant
     * Scalar reduction with no data partitioning
     * ============================================ */
    printf("Testing case 0 (gang redundant)...\n");
    {
        double sum = 0.0;
        #pragma acc parallel copyin(arr1d[0:N]) copy(sum) reduction(+:sum)
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                sum += arr1d[i];
            }
        }
        if (!verify_array(arr1d, N, sum)) {
            printf("  ERROR: Case 0 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 1: gang partitioned
     * Array data distributed across gangs only
     * ============================================ */
    printf("Testing case 1 (gang partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N])
        {
            #pragma acc loop gang
            for (int i = 0; i < N; i++) {
                arr1d[i] = arr1d[i] * 2.0;
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < N; i++) {
            expected_sum += 1.0 + i * 0.1;
        }
        expected_sum *= 2.0;
        if (!verify_array(arr1d, N, expected_sum)) {
            printf("  ERROR: Case 1 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 2: worker partitioned
     * Worker-level distribution without gangs
     * ============================================ */
    printf("Testing case 2 (worker partitioned)...\n");
    {
        #pragma acc parallel copy(arr_small[0:M]) num_workers(4)
        {
            #pragma acc loop worker
            for (int i = 0; i < M; i++) {
                arr_small[i] = arr_small[i] + 1.0;
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < M; i++) {
            expected_sum += 0.5 + i * 0.1 + 1.0;
        }
        if (!verify_array(arr_small, M, expected_sum)) {
            printf("  ERROR: Case 2 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 3: gang+worker partitioned
     * Combined gang and worker partitioning
     * ============================================ */
    printf("Testing case 3 (gang+worker partitioned)...\n");
    {
        #pragma acc parallel copy(arr2d[0:N*M])
        {
            #pragma acc loop gang worker collapse(2)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    int idx = i * M + j;
                    arr2d[idx] = arr2d[idx] / 2.0;
                }
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < N * M; i++) {
            expected_sum += 2.0 + i * 0.1;
        }
        expected_sum /= 2.0;
        if (!verify_array(arr2d, N * M, expected_sum)) {
            printf("  ERROR: Case 3 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 4: vector partitioned
     * Vector-level SIMD partitioning
     * ============================================ */
    printf("Testing case 4 (vector partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N]) vector_length(32)
        {
            #pragma acc loop vector
            for (int i = 0; i < N; i++) {
                arr1d[i] = arr1d[i] * 3.0;
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < N; i++) {
            expected_sum += (1.0 + i * 0.1) * 2.0 * 3.0;
        }
        if (!verify_array(arr1d, N, expected_sum)) {
            printf("  ERROR: Case 4 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 5: gang+vector partitioned
     * Gang and vector without workers
     * ============================================ */
    printf("Testing case 5 (gang+vector partitioned)...\n");
    {
        #pragma acc parallel copy(arr1d[0:N])
        {
            #pragma acc loop gang vector
            for (int i = 0; i < N; i++) {
                arr1d[i] = arr1d[i] + 5.0;
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < N; i++) {
            expected_sum += (1.0 + i * 0.1) * 2.0 * 3.0 + 5.0;
        }
        if (!verify_array(arr1d, N, expected_sum)) {
            printf("  ERROR: Case 5 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 6: worker+vector partitioned
     * Worker and vector without gangs
     * ============================================ */
    printf("Testing case 6 (worker+vector partitioned)...\n");
    {
        #pragma acc parallel copy(arr_small[0:M]) num_workers(2) vector_length(16)
        {
            #pragma acc loop worker vector
            for (int i = 0; i < M; i++) {
                arr_small[i] = arr_small[i] * 0.5;
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < M; i++) {
            expected_sum += (0.5 + i * 0.1 + 1.0) * 0.5;
        }
        if (!verify_array(arr_small, M, expected_sum)) {
            printf("  ERROR: Case 6 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * CASE 7: fully partitioned
     * All three levels: gang, worker, vector
     * ============================================ */
    printf("Testing case 7 (fully partitioned)...\n");
    {
        #pragma acc parallel copy(arr3d[0:N*M*P])
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    for (int k = 0; k < P; k++) {
                        int idx = (i * M + j) * P + k;
                        arr3d[idx] = arr3d[idx] - 1.0;
                    }
                }
            }
        }
        double expected_sum = 0.0;
        for (int i = 0; i < N * M * P; i++) {
            expected_sum += 3.0 + i * 0.1 - 1.0;
        }
        if (!verify_array(arr3d, N * M * P, expected_sum)) {
            printf("  ERROR: Case 7 verification failed\n");
            errors++;
        }
    }
    
    /* ============================================
     * Additional test: Mixed directives to ensure
     * compiler explores different partition paths
     * ============================================ */
    printf("Testing mixed directives...\n");
    {
        /* Test with private variables */
        double local_sum = 0.0;
        #pragma acc parallel copyin(arr1d[0:N]) copy(local_sum) private(local_sum)
        {
            double tmp = 0.0;
            #pragma acc loop gang reduction(+:tmp)
            for (int i = 0; i < N; i++) {
                tmp += arr1d[i];
            }
            local_sum = tmp;
        }
        
        /* Test with present clause (simulated) */
        #pragma acc data copy(arr1d[0:N])
        {
            #pragma acc parallel present(arr1d)
            {
                #pragma acc loop gang worker
                for (int i = 0; i < N; i++) {
                    arr1d[i] = arr1d[i] / 2.0;
                }
            }
        }
        
        /* Test triangular loop */
        #pragma acc parallel copy(arr_small[0:M])
        {
            #pragma acc loop gang
            for (int i = 0; i < M; i++) {
                #pragma acc loop worker
                for (int j = 0; j <= i; j++) {
                    arr_small[j] += 0.1;
                }
            }
        }
    }
    
    /* ============================================
     * Test with runtime parameters
     * ============================================ */
    printf("Testing with runtime parameters...\n");
    {
        int num_gangs = 4;
        int num_workers = 2;
        int vector_len = 8;
        
        #pragma acc parallel copy(arr1d[0:N]) \
                    num_gangs(num_gangs) \
                    num_workers(num_workers) \
                    vector_length(vector_len)
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < N; i++) {
                arr1d[i] = arr1d[i] + 10.0;
            }
        }
    }
    
    /* Cleanup */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(arr_small);
    
    /* Summary */
    if (errors == 0) {
        printf("\nAll partition tests passed successfully!\n");
        printf("This should trigger all 8 partition codes (0-7) in the compiler analysis.\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
