/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
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
int verify_array(double *arr, int size, double expected_sum) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return (fabs(sum - expected_sum) < 1e-6);
}

/* Test case 0: gang redundant (scalar reduction, no data partitioning across gangs) */
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

/* Test case 1: gang partitioned (array data distributed across gangs) */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 2.0);
    memset(result, 0, sizeof(double) * N);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0;
        }
    }
    
    if (verify_array(result, N, 0.0)) {  // We'll compute expected separately
        double expected_sum = 0.0;
        for (int i = 0; i < N; i++) {
            expected_sum += (2.0 + i * 0.1) * 2.0;
        }
        double actual_sum = 0.0;
        for (int i = 0; i < N; i++) {
            actual_sum += result[i];
        }
        
        if (fabs(actual_sum - expected_sum) < 1e-6) {
            printf("  PASS: gang partitioned\n");
        } else {
            printf("  FAIL: gang partitioned\n");
        }
    }
}

/* Test case 2: worker partitioned (worker-level distribution) */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[M], result[M];
    
    init_array(arr, M, 3.0);
    memset(result, 0, sizeof(double) * M);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 3.0;
        }
    }
    
    double expected_sum = 0.0;
    for (int i = 0; i < M; i++) {
        expected_sum += (3.0 + i * 0.1) * 3.0;
    }
    double actual_sum = 0.0;
    for (int i = 0; i < M; i++) {
        actual_sum += result[i];
    }
    
    if (fabs(actual_sum - expected_sum) < 1e-6) {
        printf("  PASS: worker partitioned\n");
    } else {
        printf("  FAIL: worker partitioned\n");
    }
}

/* Test case 3: gang+worker partitioned (nested gang and worker distribution) */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M], result[N][M];
    
    // Initialize 2D array
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
                result[i][j] = arr[i][j] * 2.0;
            }
        }
    }
    
    // Verify a subset of results
    int valid = 1;
    for (int i = 0; i < 10 && i < N; i++) {
        for (int j = 0; j < 10 && j < M; j++) {
            double expected = (1.0 + i * 0.01 + j * 0.001) * 2.0;
            if (fabs(result[i][j] - expected) > 1e-6) {
                valid = 0;
                break;
            }
        }
        if (!valid) break;
    }
    
    if (valid) {
        printf("  PASS: gang+worker partitioned\n");
    } else {
        printf("  FAIL: gang+worker partitioned\n");
    }
}

/* Test case 4: vector partitioned (vector-level/SIMD distribution) */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 4.0);
    memset(result, 0, sizeof(double) * N);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 4.0;
        }
    }
    
    double expected_sum = 0.0;
    for (int i = 0; i < N; i++) {
        expected_sum += (4.0 + i * 0.1) * 4.0;
    }
    double actual_sum = 0.0;
    for (int i = 0; i < N; i++) {
        actual_sum += result[i];
    }
    
    if (fabs(actual_sum - expected_sum) < 1e-6) {
        printf("  PASS: vector partitioned\n");
    } else {
        printf("  FAIL: vector partitioned\n");
    }
}

/* Test case 5: gang+vector partitioned (gang and vector without workers) */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 5.0);
    memset(result, 0, sizeof(double) * N);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    double expected_sum = 0.0;
    for (int i = 0; i < N; i++) {
        expected_sum += (5.0 + i * 0.1) * 5.0;
    }
    double actual_sum = 0.0;
    for (int i = 0; i < N; i++) {
        actual_sum += result[i];
    }
    
    if (fabs(actual_sum - expected_sum) < 1e-6) {
        printf("  PASS: gang+vector partitioned\n");
    } else {
        printf("  FAIL: gang+vector partitioned\n");
    }
}

/* Test case 6: worker+vector partitioned (worker and vector combination) */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[M], result[M];
    
    init_array(arr, M, 6.0);
    memset(result, 0, sizeof(double) * M);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    double expected_sum = 0.0;
    for (int i = 0; i < M; i++) {
        expected_sum += (6.0 + i * 0.1) * 6.0;
    }
    double actual_sum = 0.0;
    for (int i = 0; i < M; i++) {
        actual_sum += result[i];
    }
    
    if (fabs(actual_sum - expected_sum) < 1e-6) {
        printf("  PASS: worker+vector partitioned\n");
    } else {
        printf("  FAIL: worker+vector partitioned\n");
    }
}

/* Test case 7: fully partitioned (gang, worker, and vector all used) */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P], result[N][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = 1.0 + i * 0.001 + j * 0.0001 + k * 0.00001;
                result[i][j][k] = 0.0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M][0:P]) copyout(result[0:N][0:M][0:P])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] * 2.0;
                }
            }
        }
    }
    
    // Verify a small subset
    int valid = 1;
    for (int i = 0; i < 5 && i < N; i++) {
        for (int j = 0; j < 5 && j < M; j++) {
            for (int k = 0; k < 5 && k < P; k++) {
                double expected = (1.0 + i * 0.001 + j * 0.0001 + k * 0.00001) * 2.0;
                if (fabs(result[i][j][k] - expected) > 1e-6) {
                    valid = 0;
                    break;
                }
            }
            if (!valid) break;
        }
        if (!valid) break;
    }
    
    if (valid) {
        printf("  PASS: fully partitioned\n");
    } else {
        printf("  FAIL: fully partitioned\n");
    }
}

/* Test default case (if compiler testing interfaces were available) */
void test_default_case() {
    printf("Testing default case: <illegal>\n");
    printf("  Note: Default case would require compiler-internal testing hooks\n");
    printf("  This would typically be tested via unit tests, not application code\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    // Execute all test cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_default_case();
    
    printf("\nAll partition mapping tests completed.\n");
    
    return 0;
}
