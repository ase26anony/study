/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
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
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_base) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        double expected = expected_base + i * 0.1;
        if (abs(arr[i] - expected) > 1e-6) {
            errors++;
        }
    }
    return errors;
}

int main() {
    int i, j, k;
    double sum = 0.0;
    double *arr1d = (double*)malloc(N * sizeof(double));
    double *arr2d = (double*)malloc(N * M * sizeof(double));
    double *arr3d = (double*)malloc(N * M * P * sizeof(double));
    double *output = (double*)malloc(N * sizeof(double));
    
    if (!arr1d || !arr2d || !arr3d || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_array(arr1d, N, 1.0);
    init_array(arr2d, N * M, 2.0);
    init_array(arr3d, N * M * P, 3.0);
    memset(output, 0, N * sizeof(double));
    
    printf("Testing all partition cases...\n");
    
    /* ============================================
     * Case 0: gang redundant
     * Scalar reduction with no data partitioning across gangs
     * ============================================ */
    printf("Testing Case 0 (gang redundant)...\n");
    sum = 0.0;
    #pragma acc parallel copyin(arr1d[0:N]) copy(sum) reduction(+:sum)
    {
        #pragma acc loop gang
        for (i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    printf("  Sum = %f\n", sum);
    
    /* ============================================
     * Case 1: gang partitioned
     * Array data distributed across gangs but not within gangs
     * ============================================ */
    printf("Testing Case 1 (gang partitioned)...\n");
    #pragma acc parallel loop gang copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = arr1d[i] * 2.0;
    }
    
    int errors = verify_array(output, N, 2.0);
    printf("  Errors in output: %d\n", errors);
    
    /* ============================================
     * Case 2: worker partitioned
     * Worker-level distribution
     * ============================================ */
    printf("Testing Case 2 (worker partitioned)...\n");
    #pragma acc parallel loop worker num_workers(4) copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = arr1d[i] + 1.0;
    }
    
    errors = verify_array(output, N, 2.0);
    printf("  Errors in output: %d\n", errors);
    
    /* ============================================
     * Case 3: gang+worker partitioned
     * Combined gang and worker partitioning with nested loops
     * ============================================ */
    printf("Testing Case 3 (gang+worker partitioned)...\n");
    #pragma acc parallel loop gang worker collapse(2) copy(arr2d[0:N*M]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            int idx = i * M + j;
            #pragma acc atomic
            output[i] += arr2d[idx];
        }
    }
    
    /* ============================================
     * Case 4: vector partitioned
     * Vector-level partitioning for SIMD-style operations
     * ============================================ */
    printf("Testing Case 4 (vector partitioned)...\n");
    #pragma acc parallel loop vector vector_length(32) copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = arr1d[i] * arr1d[i];
    }
    
    /* ============================================
     * Case 5: gang+vector partitioned
     * Combine gang and vector partitioning without workers
     * ============================================ */
    printf("Testing Case 5 (gang+vector partitioned)...\n");
    #pragma acc parallel loop gang vector copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = sin(arr1d[i]) + cos(arr1d[i]);
    }
    
    /* ============================================
     * Case 6: worker+vector partitioned
     * Combine worker and vector partitioning
     * ============================================ */
    printf("Testing Case 6 (worker+vector partitioned)...\n");
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) \
                copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = arr1d[i] * 3.0 - 1.0;
    }
    
    errors = verify_array(output, N, 2.0);
    printf("  Errors in output: %d\n", errors);
    
    /* ============================================
     * Case 7: fully partitioned
     * Use all three levels: gang, worker, vector
     * ============================================ */
    printf("Testing Case 7 (fully partitioned)...\n");
    #pragma acc parallel loop gang worker vector collapse(3) \
                copy(arr3d[0:N*M*P]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            for (k = 0; k < P; k++) {
                int idx = (i * M * P) + (j * P) + k;
                #pragma acc atomic
                output[i] += arr3d[idx] * 0.01;
            }
        }
    }
    
    /* ============================================
     * Additional tests with different data mappings
     * to influence partitioning decisions
     * ============================================ */
    
    /* Test with present clause (simulating already resident data) */
    printf("Testing with present clause...\n");
    #pragma acc enter data copyin(arr1d[0:N])
    #pragma acc parallel loop gang present(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = arr1d[i] * 4.0;
    }
    #pragma acc exit data delete(arr1d[0:N])
    
    /* Test with private variables */
    printf("Testing with private variables...\n");
    #pragma acc parallel loop gang private(j) copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        double temp = 0.0;
        for (j = 0; j < 10; j++) {
            temp += arr1d[i] * j;
        }
        output[i] = temp;
    }
    
    /* Test with runtime parameters */
    printf("Testing with runtime parameters...\n");
    int num_gangs = 8;
    int num_workers = 2;
    int vector_len = 16;
    
    #pragma acc parallel loop gang num_gangs(num_gangs) \
                worker num_workers(num_workers) \
                vector vector_length(vector_len) \
                copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = arr1d[i] / 2.0;
    }
    
    /* Test triangular loop (non-rectangular iteration space) */
    printf("Testing triangular loop...\n");
    #pragma acc parallel loop gang copy(arr1d[0:N]) copyout(output[0:N])
    for (i = 0; i < N; i++) {
        output[i] = 0.0;
        #pragma acc loop worker reduction(+:output[i])
        for (j = 0; j <= i; j++) {
            output[i] += arr1d[j];
        }
    }
    
    /* Clean up */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(output);
    
    printf("\nAll partition tests completed.\n");
    printf("To trigger the default case (<illegal>), the compiler's internal\n");
    printf("partitioning logic would need to receive values outside 0-7 range.\n");
    printf("This typically requires compiler debugging hooks or malformed IR.\n");
    
    return 0;
}
