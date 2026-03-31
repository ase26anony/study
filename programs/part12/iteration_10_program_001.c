/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or with OpenMP: gcc -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define N 1024
#define M 128
#define P 64

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double start_val) {
    for (int i = 0; i < size; i++) {
        arr[i] = start_val + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_sum) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return fabs(sum - expected_sum) < 1e-6;
}

/* Test case 0: gang redundant - scalar reduction */
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
    
    double expected = N * 1.0 + 0.1 * (N-1) * N / 2;
    if (fabs(sum - expected) < 1e-6) {
        printf("  PASS: sum = %f\n", sum);
    } else {
        printf("  FAIL: expected %f, got %f\n", expected, sum);
    }
}

/* Test case 1: gang partitioned - array distributed across gangs */
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
    
    if (verify_array(result, N, 2.0 * (N * 2.0 + 0.1 * (N-1) * N / 2))) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test case 2: worker partitioned */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[M], result[M];
    
    init_array(arr, M, 3.0);
    memset(result, 0, M * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = sin(arr[i]) + cos(arr[i]);
        }
    }
    
    // Simple verification
    int valid = 1;
    for (int i = 0; i < M; i++) {
        double expected = sin(arr[i]) + cos(arr[i]);
        if (fabs(result[i] - expected) > 1e-6) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test case 3: gang+worker partitioned */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M], result[N][M];
    
    // Initialize 2D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * 100.0 + j * 0.1;
            result[i][j] = 0.0;
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M]) copyout(result[0:N][0:M])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = arr[i][j] * 1.5;
            }
        }
    }
    
    // Verification
    int valid = 1;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            double expected = arr[i][j] * 1.5;
            if (fabs(result[i][j] - expected) > 1e-6) {
                valid = 0;
                break;
            }
        }
        if (!valid) break;
    }
    
    if (valid) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test case 4: vector partitioned */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float arr[N], result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = i * 0.01f;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * arr[i];
        }
    }
    
    int valid = 1;
    for (int i = 0; i < N; i++) {
        float expected = arr[i] * arr[i];
        if (fabs(result[i] - expected) > 1e-6) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test case 5: gang+vector partitioned */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 5.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = exp(arr[i] * 0.01);
        }
    }
    
    int valid = 1;
    for (int i = 0; i < N; i++) {
        double expected = exp(arr[i] * 0.01);
        if (fabs(result[i] - expected) > 1e-6) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test case 6: worker+vector partitioned */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[M], result[M];
    
    for (int i = 0; i < M; i++) {
        arr[i] = i * 0.1f;
        result[i] = 0.0f;
    }
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = sqrtf(arr[i] + 1.0f);
        }
    }
    
    int valid = 1;
    for (int i = 0; i < M; i++) {
        float expected = sqrtf(arr[i] + 1.0f);
        if (fabs(result[i] - expected) > 1e-6) {
            valid = 0;
            break;
        }
    }
    
    if (valid) {
        printf("  PASS\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test case 7: fully partitioned (gang+worker+vector) */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P];
    double result[N][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 10000.0 + j * 100.0 + k * 1.0;
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
                    result[i][j][k] = arr[i][j][k] * 0.5;
                }
            }
        }
    }
    
    // Simple spot check
    int valid = 1;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                double expected = arr[i][j][k] * 0.5;
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
        printf("  PASS (spot check)\n");
    } else {
        printf("  FAIL\n");
    }
}

/* Test default case - using runtime parameters to potentially trigger edge cases */
void test_edge_cases() {
    printf("Testing edge cases (potential default path triggers)\n");
    
    // Variable loop bounds that might affect partitioning decisions
    int n = 100;
    double arr[100], result[100];
    
    for (int i = 0; i < n; i++) {
        arr[i] = i * 0.5;
        result[i] = 0.0;
    }
    
    // Mixed directives that might produce unusual partition patterns
    #pragma acc parallel copyin(arr[0:n]) copyout(result[0:n])
    {
        #pragma acc loop independent
        for (int i = 0; i < n; i++) {
            result[i] = arr[i] * 2.0;
        }
    }
    
    // Test with private variables
    double private_var = 0.0;
    #pragma acc parallel copyin(arr[0:n]) private(private_var)
    {
        #pragma acc loop
        for (int i = 0; i < n; i++) {
            private_var = arr[i];
            // Do something with private_var
        }
    }
    
    printf("  Edge cases executed (default path would need compiler internals)\n");
}

int main() {
    printf("=== Testing all partition mapping cases ===\n\n");
    
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_edge_cases();
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
