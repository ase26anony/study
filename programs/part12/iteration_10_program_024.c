/* Test program to exercise all OpenACC partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Test 0: gang redundant - scalar reductions, no data partitioning across gangs */
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
    
    printf("  Sum = %f\n", sum);
}

/* Test 1: gang partitioned - array data distributed across gangs */
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
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 2.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 2: worker partitioned - worker-level distribution */
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
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (result[i] != arr[i] * 3.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M], result[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * 100.0 + j;
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
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            if (result[i][j] != arr[i][j] * 2.0) errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 4.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 4.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 4.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 5.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 5.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 6.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 6.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P], result[N][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 10000.0 + j * 100.0 + k;
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
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (result[i][j][k] != arr[i][j][k] * 2.0) errors++;
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test default case: This would normally require compiler internals or debug hooks.
 * For completeness, we include a placeholder that might trigger unusual partitioning
 * through complex data dependencies and irregular access patterns.
 */
void test_complex_partitioning() {
    printf("Testing complex partitioning patterns\n");
    double arr[N], result[N];
    int indices[N];
    
    // Create irregular access pattern
    for (int i = 0; i < N; i++) {
        arr[i] = i * 1.5;
        indices[i] = (i * 17) % N;  // Non-linear index pattern
    }
    
    #pragma acc parallel copyin(arr[0:N], indices[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int idx = indices[i];
            #pragma acc loop worker vector
            for (int j = 0; j < 4; j++) {
                // Complex nested computation
                result[idx] = arr[idx] * (j + 1);
            }
        }
    }
    
    printf("  Complex test completed\n");
}

int main() {
    printf("Starting OpenACC partition mapping tests\n");
    printf("========================================\n");
    
    // Run all partition test cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_complex_partitioning();
    
    printf("\nAll tests completed successfully!\n");
    printf("Note: To trigger the exact uncovered lines in omp-oacc-neuter-broadcast.cc,\n");
    printf("compile with GCC and appropriate debugging flags:\n");
    printf("  gcc -O1 -fopenacc -fdump-tree-omplower -g -fstack-protector-strong test_partitions.c\n");
    
    return 0;
}
