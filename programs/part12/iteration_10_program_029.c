/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * 
 * Compilation options:
 *   OpenACC: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 *   OpenMP:  gcc -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -o test_partitions test_partitions.c
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
    
    double expected = 0.0;
    for (int i = 0; i < N; i++) {
        expected += 1.0 + i * 0.1;
    }
    
    if (fabs(sum - expected) < 1e-6) {
        printf("  PASS: gang redundant reduction\n");
    } else {
        printf("  FAIL: gang redundant reduction (got %f, expected %f)\n", sum, expected);
    }
}

/* Test Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 2.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0;
        }
    }
    
    if (verify_array(result, N, 2.0 * verify_array(arr, N, 0.0))) {
        printf("  PASS: gang partitioned array operation\n");
    } else {
        printf("  FAIL: gang partitioned array operation\n");
    }
}

/* Test Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    
    double arr[M], result[M];
    init_array(arr, M, 3.0);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 3.0;
        }
    }
    
    if (verify_array(result, M, 3.0 * verify_array(arr, M, 0.0))) {
        printf("  PASS: worker partitioned array operation\n");
    } else {
        printf("  FAIL: worker partitioned array operation\n");
    }
}

/* Test Case 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    double arr[M][M], result[M][M];
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = 1.0 + i * 0.01 + j * 0.001;
            result[i][j] = 0.0;
        }
    }
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = arr[i][j] * 4.0;
            }
        }
    }
    
    double sum = 0.0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            sum += result[i][j];
        }
    }
    
    printf("  INFO: gang+worker partitioned 2D array (sum = %f)\n", sum);
}

/* Test Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 5.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    if (verify_array(result, N, 5.0 * verify_array(arr, N, 0.0))) {
        printf("  PASS: vector partitioned array operation\n");
    } else {
        printf("  FAIL: vector partitioned array operation\n");
    }
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 6.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    if (verify_array(result, N, 6.0 * verify_array(arr, N, 0.0))) {
        printf("  PASS: gang+vector partitioned array operation\n");
    } else {
        printf("  FAIL: gang+vector partitioned array operation\n");
    }
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 7.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 7.0;
        }
    }
    
    if (verify_array(result, N, 7.0 * verify_array(arr, N, 0.0))) {
        printf("  PASS: worker+vector partitioned array operation\n");
    } else {
        printf("  FAIL: worker+vector partitioned array operation\n");
    }
}

/* Test Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    double arr[P][M][M], result[P][M][M];
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                arr[i][j][k] = 1.0 + i * 0.1 + j * 0.01 + k * 0.001;
                result[i][j][k] = 0.0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < M; k++) {
                    result[i][j][k] = arr[i][j][k] * 8.0;
                }
            }
        }
    }
    
    double sum = 0.0;
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < M; k++) {
                sum += result[i][j][k];
            }
        }
    }
    
    printf("  INFO: fully partitioned 3D array (sum = %f)\n", sum);
}

/* Test default case - if compiler testing interfaces are available */
void test_default_case() {
    printf("Testing default case (if supported by compiler)\n");
    
    /* This would normally require compiler-specific testing hooks
     * For demonstration, we show what might trigger edge cases */
    
    /* Using variable loop bounds that might affect partitioning */
    int dynamic_size = N;
    double arr[N];
    init_array(arr, N, 9.0);
    
    #pragma acc parallel copy(arr[0:dynamic_size])
    {
        #pragma acc loop
        for (int i = 0; i < dynamic_size; i++) {
            arr[i] *= 2.0;
        }
    }
    
    printf("  INFO: dynamic size partitioning test completed\n");
}

/* Main function executing all test cases */
int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_default_case();
    
    printf("\nAll partition tests completed.\n");
    printf("To verify coverage, compile with:\n");
    printf("  gcc -O1 -fopenacc -fdump-tree-omplower -g -fstack-protector-strong test_partitions.c\n");
    printf("And check the generated tree dumps for partition code usage.\n");
    
    return 0;
}
