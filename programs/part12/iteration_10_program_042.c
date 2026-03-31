/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition string mappings
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
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
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
    
    printf("  Sum: %f\n", sum);
}

/* Test case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N], result[N];
    init_array(arr, N, 2.0);
    
    #pragma acc parallel loop gang copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * 2.0;
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (abs(result[i] - (2.0 + i*0.1)*2.0) > 0.0001) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[M];
    init_array(arr, M, 3.0);
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = arr[i] * 3.0;
    }
    
    printf("  First element: %f\n", arr[0]);
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double matrix[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = i * 100.0 + j;
        }
    }
    
    #pragma acc parallel loop gang worker collapse(2) copy(matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = matrix[i][j] * 1.5;
        }
    }
    
    printf("  Center element: %f\n", matrix[N/2][M/2]);
}

/* Test case 4: vector partitioned - SIMD/vector-level operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float vec[N];
    for (int i = 0; i < N; i++) {
        vec[i] = i * 1.5f;
    }
    
    #pragma acc parallel loop vector vector_length(32) copy(vec[0:N])
    for (int i = 0; i < N; i++) {
        vec[i] = vec[i] * 2.0f;
    }
    
    printf("  First/last: %f, %f\n", vec[0], vec[N-1]);
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double data[N];
    init_array(data, N, 5.0);
    
    #pragma acc parallel loop gang vector copy(data[0:N])
    for (int i = 0; i < N; i++) {
        data[i] = data[i] + i * 0.01;
    }
    
    printf("  Sum of first 10: ");
    double sum = 0;
    for (int i = 0; i < 10; i++) sum += data[i];
    printf("%f\n", sum);
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    float arr[M];
    for (int i = 0; i < M; i++) {
        arr[i] = i * 2.0f;
    }
    
    #pragma acc parallel loop worker vector num_workers(2) vector_length(16) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = sinf(arr[i]) + cosf(arr[i]);
    }
    
    printf("  Processed %d elements\n", M);
}

/* Test case 7: fully partitioned - all three levels (gang, worker, vector) */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double cube[P][M][N];
    
    // Initialize 3D array
    #pragma acc parallel loop gang worker vector collapse(3) copyout(cube[0:P][0:M][0:N])
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                cube[i][j][k] = i * 10000.0 + j * 100.0 + k;
            }
        }
    }
    
    // Process with full partitioning
    #pragma acc parallel loop gang worker vector collapse(3) copy(cube[0:P][0:M][0:N])
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                cube[i][j][k] = cube[i][j][k] * 0.5;
            }
        }
    }
    
    printf("  Corner values: %f, %f\n", cube[0][0][0], cube[P-1][M-1][N-1]);
}

/* Additional test to potentially trigger default case through edge conditions */
void test_edge_cases() {
    printf("Testing edge cases (may trigger default path)\n");
    
    // Variable loop bounds can affect partitioning decisions
    int dynamic_size = N / 2;
    double dyn_arr[dynamic_size];
    
    #pragma acc parallel loop gang copy(dyn_arr[0:dynamic_size])
    for (int i = 0; i < dynamic_size; i++) {
        dyn_arr[i] = i * 1.0;
    }
    
    // Triangular loop pattern
    #pragma acc parallel loop gang
    for (int i = 0; i < N; i++) {
        #pragma acc loop worker
        for (int j = 0; j < i; j++) {
            // Non-rectangular access pattern
        }
    }
    
    printf("  Edge cases completed\n");
}

int main() {
    printf("=== Testing all partition mapping cases ===\n\n");
    
    // Execute all test cases to trigger different partition codes
    test_gang_redundant();           // Should trigger case 0
    test_gang_partitioned();         // Should trigger case 1
    test_worker_partitioned();       // Should trigger case 2
    test_gang_worker_partitioned();  // Should trigger case 3
    test_vector_partitioned();       // Should trigger case 4
    test_gang_vector_partitioned();  // Should trigger case 5
    test_worker_vector_partitioned();// Should trigger case 6
    test_fully_partitioned();        // Should trigger case 7
    test_edge_cases();               // Additional edge cases
    
    printf("\n=== All tests completed ===\n");
    
    // For compiler testing/debugging, we could also directly test the mapping function
    // if the compiler provides testing hooks (not typically available in user code)
    
    return 0;
}
