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
#define P 16

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Test case 0: gang redundant - scalar reductions, no data partitioning across gangs */
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

/* Test case 1: gang partitioned - array data distributed across gangs */
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

/* Test case 2: worker partitioned - worker-level distribution */
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

/* Test case 3: gang+worker partitioned - nested gang and worker distribution */
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

/* Test case 4: vector partitioned - vector-level SIMD operations */
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

/* Test case 5: gang+vector partitioned - gang and vector without workers */
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

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 6.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(32)
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

/* Test case 7: fully partitioned - gang, worker, and vector all active */
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

/* Additional test with runtime parameters to influence partitioning decisions */
void test_runtime_partitioning() {
    printf("Testing runtime partitioning decisions\n");
    
    int size = 512;
    double *arr = (double*)malloc(size * sizeof(double));
    double *result = (double*)malloc(size * sizeof(double));
    
    init_array(arr, size, 8.0);
    
    // Use runtime variable for vector length
    int vec_len = 16;
    
    #pragma acc parallel copyin(arr[0:size]) copyout(result[0:size]) \
                num_gangs(size/64) num_workers(4) vector_length(vec_len)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < size; i++) {
            result[i] = arr[i] * 8.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (result[i] != arr[i] * 8.0) errors++;
    }
    printf("  Runtime partitioning errors: %d\n", errors);
    
    free(arr);
    free(result);
}

/* Test with different data clauses to influence partitioning */
void test_data_clauses() {
    printf("Testing various data clauses\n");
    
    double arr1[N], arr2[N], arr3[N];
    double sum = 0.0;
    
    init_array(arr1, N, 1.0);
    init_array(arr2, N, 2.0);
    
    // Test with copy, private, and reduction clauses
    #pragma acc parallel copy(arr1[0:N]) copyin(arr2[0:N]) copyout(arr3[0:N]) \
                private(sum) reduction(+:sum)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            double temp = arr1[i] + arr2[i];
            arr3[i] = temp;
            sum += temp;
        }
    }
    
    printf("  Data clauses test sum: %f\n", sum);
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    // Execute all test cases to trigger different partition codes
    test_gang_redundant();           // Should trigger case 0
    test_gang_partitioned();         // Should trigger case 1
    test_worker_partitioned();       // Should trigger case 2
    test_gang_worker_partitioned();  // Should trigger case 3
    test_vector_partitioned();       // Should trigger case 4
    test_gang_vector_partitioned();  // Should trigger case 5
    test_worker_vector_partitioned();// Should trigger case 6
    test_fully_partitioned();        // Should trigger case 7
    
    // Additional tests to cover edge cases
    test_runtime_partitioning();
    test_data_clauses();
    
    printf("\nAll tests completed.\n");
    printf("Note: To trigger the default case (case 8+), you would need to:\n");
    printf("1. Use compiler internal testing hooks (if available)\n");
    printf("2. Pass invalid partition codes through debug interfaces\n");
    printf("3. Use malformed directives that produce unexpected values\n");
    
    return 0;
}
