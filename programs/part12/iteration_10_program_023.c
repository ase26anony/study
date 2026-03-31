/* Test program to exercise all OpenACC partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

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
    double *arr1d = (double*)malloc(N * sizeof(double));
    double *arr2d = (double*)malloc(N * M * sizeof(double));
    double *arr3d = (double*)malloc(N * M * P * sizeof(double));
    double sum = 0.0;
    int errors = 0;
    
    if (!arr1d || !arr2d || !arr3d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    printf("Testing OpenACC partition mapping cases...\n");
    
    /* ============================================
     * Case 0: gang redundant
     * Scalar reduction with no data partitioning across gangs
     * Should trigger "gang redundant" partition code (0)
     * ============================================ */
    printf("Test 0: gang redundant (scalar reduction)...\n");
    init_array(arr1d, N, 1.0);
    sum = 0.0;
    
    #pragma acc parallel copyin(arr1d[0:N]) reduction(+:sum) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr1d[i];
        }
    }
    
    double expected_sum = 0.0;
    for (int i = 0; i < N; i++) {
        expected_sum += 1.0 + i * 0.1;
    }
    if (abs(sum - expected_sum) > 1e-6) {
        printf("  ERROR: sum mismatch (got %f, expected %f)\n", sum, expected_sum);
        errors++;
    }
    
    /* ============================================
     * Case 1: gang partitioned
     * Array data distributed across gangs but not within gangs
     * Should trigger "gang partitioned" partition code (1)
     * ============================================ */
    printf("Test 1: gang partitioned (array across gangs)...\n");
    init_array(arr1d, N, 2.0);
    
    #pragma acc parallel loop gang copy(arr1d[0:N]) num_gangs(8)
    for (int i = 0; i < N; i++) {
        arr1d[i] *= 2.0;
    }
    
    errors += verify_array(arr1d, N, 4.0);
    
    /* ============================================
     * Case 2: worker partitioned
     * Worker-level distribution without gang partitioning
     * Should trigger "worker partitioned" partition code (2)
     * ============================================ */
    printf("Test 2: worker partitioned (worker-level)...\n");
    init_array(arr1d, N, 3.0);
    
    #pragma acc parallel loop worker copy(arr1d[0:N]) num_workers(4)
    for (int i = 0; i < N; i++) {
        arr1d[i] += i;
    }
    
    /* ============================================
     * Case 3: gang+worker partitioned
     * Combined gang and worker partitioning with nested loops
     * Should trigger "gang+worker partitioned" partition code (3)
     * ============================================ */
    printf("Test 3: gang+worker partitioned (2D array)...\n");
    init_array(arr2d, N * M, 4.0);
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr2d[0:N*M]) \
        num_gangs(4) num_workers(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            arr2d[idx] = arr2d[idx] * (i + j);
        }
    }
    
    /* ============================================
     * Case 4: vector partitioned
     * Vector-level partitioning for SIMD operations
     * Should trigger "vector partitioned" partition code (4)
     * ============================================ */
    printf("Test 4: vector partitioned (SIMD-style)...\n");
    init_array(arr1d, N, 5.0);
    
    #pragma acc parallel loop vector copy(arr1d[0:N]) vector_length(32)
    for (int i = 0; i < N; i++) {
        arr1d[i] = sin(arr1d[i]) + cos(arr1d[i]);
    }
    
    /* ============================================
     * Case 5: gang+vector partitioned
     * Gang and vector partitioning without workers
     * Should trigger "gang+vector partitioned" partition code (5)
     * ============================================ */
    printf("Test 5: gang+vector partitioned (stride-1 access)...\n");
    init_array(arr1d, N, 6.0);
    
    #pragma acc parallel loop gang vector copy(arr1d[0:N]) \
        num_gangs(4) vector_length(64)
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] * arr1d[i];
    }
    
    errors += verify_array(arr1d, N, 36.0);
    
    /* ============================================
     * Case 6: worker+vector partitioned
     * Worker and vector partitioning without gangs
     * Should trigger "worker+vector partitioned" partition code (6)
     * ============================================ */
    printf("Test 6: worker+vector partitioned...\n");
    init_array(arr1d, N, 7.0);
    
    #pragma acc parallel loop worker vector copy(arr1d[0:N]) \
        num_workers(2) vector_length(16)
    for (int i = 0; i < N; i++) {
        arr1d[i] = sqrt(arr1d[i]);
    }
    
    /* ============================================
     * Case 7: fully partitioned
     * All three levels: gang, worker, and vector
     * Should trigger "fully partitioned" partition code (7)
     * ============================================ */
    printf("Test 7: fully partitioned (3D array)...\n");
    init_array(arr3d, N * M * P, 8.0);
    
    #pragma acc parallel loop gang worker vector collapse(3) \
        copy(arr3d[0:N*M*P]) num_gangs(2) num_workers(2) vector_length(8)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                int idx = (i * M + j) * P + k;
                arr3d[idx] = arr3d[idx] / (i + j + k + 1);
            }
        }
    }
    
    /* ============================================
     * Additional tests with different data clauses
     * to influence partitioning decisions
     * ============================================ */
    printf("Test 8: Mixed data clauses (present/original)...\n");
    double *host_arr = (double*)malloc(N * sizeof(double));
    init_array(host_arr, N, 9.0);
    
    /* Allocate on device */
    #pragma acc enter data copyin(host_arr[0:N])
    
    /* Use present clause */
    #pragma acc parallel loop present(host_arr[0:N]) num_gangs(4)
    for (int i = 0; i < N; i++) {
        host_arr[i] += 1.0;
    }
    
    /* Retrieve data */
    #pragma acc exit data copyout(host_arr[0:N])
    
    errors += verify_array(host_arr, N, 10.0);
    
    /* ============================================
     * Test with private variables
     * ============================================ */
    printf("Test 9: Private variables...\n");
    init_array(arr1d, N, 10.0);
    double private_scalar = 5.0;
    
    #pragma acc parallel loop copy(arr1d[0:N]) private(private_scalar) \
        num_gangs(2) num_workers(2)
    for (int i = 0; i < N; i++) {
        double local_var = private_scalar + i;
        arr1d[i] += local_var;
    }
    
    /* ============================================
     * Test with runtime parameters
     * ============================================ */
    printf("Test 10: Runtime parameters...\n");
    init_array(arr1d, N, 11.0);
    int runtime_gangs = 2;
    int runtime_workers = 4;
    int runtime_vector = 16;
    
    #pragma acc parallel loop gang worker vector copy(arr1d[0:N]) \
        num_gangs(runtime_gangs) num_workers(runtime_workers) \
        vector_length(runtime_vector)
    for (int i = 0; i < N; i++) {
        arr1d[i] = log(arr1d[i] + 1.0);
    }
    
    /* ============================================
     * Test array reductions (different partitioning)
     * ============================================ */
    printf("Test 11: Array reduction...\n");
    init_array(arr1d, N, 12.0);
    double partial_sums[4] = {0.0, 0.0, 0.0, 0.0};
    
    #pragma acc parallel loop gang copyin(arr1d[0:N]) copy(partial_sums[0:4]) \
        num_gangs(4)
    for (int i = 0; i < N; i++) {
        int gang_id = 0; /* Would normally get actual gang ID */
        partial_sums[gang_id % 4] += arr1d[i];
    }
    
    /* ============================================
     * Test triangular loop (non-rectangular)
     * ============================================ */
    printf("Test 12: Triangular loop...\n");
    init_array(arr1d, N, 13.0);
    
    #pragma acc parallel loop gang copy(arr1d[0:N])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < i; j++) {
            arr1d[i] += 0.01;
        }
    }
    
    /* ============================================
     * Cleanup and final report
     * ============================================ */
    free(arr1d);
    free(arr2d);
    free(arr3d);
    free(host_arr);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("All partition mapping cases (0-7) should have been exercised.\n");
        printf("Note: The default case (8+) requires compiler-internal testing\n");
        printf("      with invalid partition codes via debug interfaces.\n");
        return 0;
    } else {
        printf("\n%d verification errors detected.\n", errors);
        return 1;
    }
}
