/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Test 0: gang redundant - scalar reduction, no data partitioning across gangs */
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
    printf("  Sum: %f\n", sum);
}

/* Test 1: gang partitioned - array distributed across gangs only */
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
    double arr[N], result[N];
    init_array(arr, N, 3.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 3.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 3.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[M][N], result[M][N];
    
    // Initialize 2D array
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = i * 100.0 + j * 0.1;
        }
    }
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                result[i][j] = arr[i][j] * 2.0;
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
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
    double arr[M][N][P], result[M][N][P];
    
    // Initialize 3D array
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 10000.0 + j * 100.0 + k * 1.0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] * 2.0;
                }
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < P; k++) {
                if (result[i][j][k] != arr[i][j][k] * 2.0) errors++;
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test default case - if compiler testing interfaces are available */
void test_default_case() {
    printf("Testing default case (if possible)\n");
    /* Note: Directly testing the default case requires compiler internals.
     * This would typically be done through compiler-specific test hooks
     * or by passing invalid partition codes to internal functions.
     * For a standalone test, we'll just note this limitation.
     */
    printf("  Default case testing requires compiler internals\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    // Run all partition type tests
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Note about default case
    test_default_case();
    
    printf("\nAll partition tests completed.\n");
    printf("To ensure full coverage of lines 335-343 in omp-oacc-neuter-broadcast.cc:\n");
    printf("1. Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -fdump-tree-omplower\n");
    printf("2. Check the tree dump for partition code generation\n");
    printf("3. Look for calls to the partition mapping function with values 0-7\n");
    
    return 0;
}
