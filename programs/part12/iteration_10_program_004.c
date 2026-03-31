/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or with OpenMP: gcc -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 64

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
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

/* Test Case 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    double sum = 0.0;
    double arr[N];
    
    init_array(arr, N, 1.0);
    
    #pragma acc parallel copyin(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    double expected_sum = 0.0;
    for (int i = 0; i < N; i++) {
        expected_sum += 1.0 + i * 0.1;
    }
    
    if (fabs(sum - expected_sum) < 1e-6) {
        printf("  PASS: gang redundant reduction\n");
    } else {
        printf("  FAIL: gang redundant reduction\n");
    }
}

/* Test Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 2.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0;
        }
    }
    
    if (verify_array(result, N, 2.0 * (2.0 * N + 0.1 * N * (N-1) / 2))) {
        printf("  PASS: gang partitioned\n");
    } else {
        printf("  FAIL: gang partitioned\n");
    }
}

/* Test Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[M], result[M];
    
    init_array(arr, M, 3.0);
    memset(result, 0, M * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 3.0;
        }
    }
    
    if (verify_array(result, M, 3.0 * (3.0 * M + 0.1 * M * (M-1) / 2))) {
        printf("  PASS: worker partitioned\n");
    } else {
        printf("  FAIL: worker partitioned\n");
    }
}

/* Test Case 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M], result[N][M];
    
    /* Initialize 2D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = 1.0 + i * 0.01 + j * 0.001;
            result[i][j] = 0.0;
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M]) copyout(result[0:N][0:M])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = arr[i][j] * 4.0;
            }
        }
    }
    
    /* Verify a subset to avoid expensive CPU computation */
    double checksum = 0.0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            checksum += result[i][j];
        }
    }
    
    printf("  CHECK: gang+worker partitioned checksum = %f\n", checksum);
}

/* Test Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 5.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    if (verify_array(result, N, 5.0 * (5.0 * N + 0.1 * N * (N-1) / 2))) {
        printf("  PASS: vector partitioned\n");
    } else {
        printf("  FAIL: vector partitioned\n");
    }
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 6.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    if (verify_array(result, N, 6.0 * (6.0 * N + 0.1 * N * (N-1) / 2))) {
        printf("  PASS: gang+vector partitioned\n");
    } else {
        printf("  FAIL: gang+vector partitioned\n");
    }
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[M], result[M];
    
    init_array(arr, M, 7.0);
    memset(result, 0, M * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 7.0;
        }
    }
    
    if (verify_array(result, M, 7.0 * (7.0 * M + 0.1 * M * (M-1) / 2))) {
        printf("  PASS: worker+vector partitioned\n");
    } else {
        printf("  FAIL: worker+vector partitioned\n");
    }
}

/* Test Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P];
    double result[N][M][P];
    
    /* Initialize 3D array with small values */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                arr[i][j][k] = 1.0 + i * 0.1 + j * 0.01 + k * 0.001;
                result[i][j][k] = 0.0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:4][0:4][0:4]) copyout(result[0:4][0:4][0:4])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    result[i][j][k] = arr[i][j][k] * 8.0;
                }
            }
        }
    }
    
    /* Verify a small subset */
    double checksum = 0.0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                checksum += result[i][j][k];
            }
        }
    }
    
    printf("  CHECK: fully partitioned checksum = %f\n", checksum);
}

/* Test default case - if compiler testing interfaces are available */
void test_default_case() {
    printf("Testing default case (if possible)\n");
    printf("  Note: Default case typically requires compiler-internal testing hooks\n");
    printf("  or malformed directives that produce invalid partition codes\n");
    
    /* Attempt to create ambiguous partitioning scenario */
    double arr[10];
    init_array(arr, 10, 9.0);
    
    /* Complex loop with multiple clauses that might confuse partitioning analysis */
    #pragma acc parallel copy(arr[0:10])
    {
        /* Mixed gang/worker/vector clauses without clear hierarchy */
        #pragma acc loop gang worker vector
        for (int i = 0; i < 10; i++) {
            arr[i] = arr[i] * 2.0;
        }
    }
    
    printf("  CHECK: ambiguous partitioning test completed\n");
}

int main() {
    printf("=== Testing all partition mapping cases ===\n\n");
    
    /* Execute all test cases */
    test_gang_redundant();
    printf("\n");
    
    test_gang_partitioned();
    printf("\n");
    
    test_worker_partitioned();
    printf("\n");
    
    test_gang_worker_partitioned();
    printf("\n");
    
    test_vector_partitioned();
    printf("\n");
    
    test_gang_vector_partitioned();
    printf("\n");
    
    test_worker_vector_partitioned();
    printf("\n");
    
    test_fully_partitioned();
    printf("\n");
    
    test_default_case();
    printf("\n");
    
    printf("=== All partition tests completed ===\n");
    
    return 0;
}
