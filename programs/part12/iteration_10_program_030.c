/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition string mappings
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 16

/* Helper to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Test 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    double sum = 0.0;
    double scalar = 2.5;
    
    #pragma acc parallel copyin(scalar) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += scalar * 0.5;
        }
    }
    printf("  Result: sum = %f\n", sum);
}

/* Test 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N];
    init_array(arr, N, 1.0);
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * 2.0;
    }
    
    double check = 0.0;
    for (int i = 0; i < N; i++) check += arr[i];
    printf("  Result: array sum = %f\n", check);
}

/* Test 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[M];
    init_array(arr, M, 2.0);
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = arr[i] / 2.0;
    }
    
    double check = 0.0;
    for (int i = 0; i < M; i++) check += arr[i];
    printf("  Result: array sum = %f\n", check);
}

/* Test 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * 100.0 + j;
        }
    }
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = arr[i][j] * 0.5;
        }
    }
    
    double check = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            check += arr[i][j];
        }
    }
    printf("  Result: 2D array sum = %f\n", check);
}

/* Test 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    double arr[N];
    init_array(arr, N, 3.0);
    
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] + 1.0;
    }
    
    double check = 0.0;
    for (int i = 0; i < N; i++) check += arr[i];
    printf("  Result: array sum = %f\n", check);
}

/* Test 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N];
    init_array(arr, N, 4.0);
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * arr[i];
    }
    
    double check = 0.0;
    for (int i = 0; i < N; i++) check += arr[i];
    printf("  Result: array sum = %f\n", check);
}

/* Test 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[M];
    init_array(arr, M, 5.0);
    
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = arr[i] - 1.0;
    }
    
    double check = 0.0;
    for (int i = 0; i < M; i++) check += arr[i];
    printf("  Result: array sum = %f\n", check);
}

/* Test 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * 10000.0 + j * 100.0 + k;
            }
        }
    }
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = arr[i][j][k] * 0.1;
            }
        }
    }
    
    double check = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                check += arr[i][j][k];
            }
        }
    }
    printf("  Result: 3D array sum = %f\n", check);
}

/* Test default case - if compiler testing interfaces were available */
void test_default_case() {
    printf("Testing default case (if compiler hooks available)\n");
    printf("  Note: Would require compiler-internal testing interfaces\n");
    printf("  to pass invalid partition codes (e.g., -1, 8, 255)\n");
}

int main() {
    printf("=== Testing all partition mapping cases ===\n\n");
    
    // Execute all partition scenarios
    test_gang_redundant();           // Case 0
    test_gang_partitioned();         // Case 1
    test_worker_partitioned();       // Case 2
    test_gang_worker_partitioned();  // Case 3
    test_vector_partitioned();       // Case 4
    test_gang_vector_partitioned();  // Case 5
    test_worker_vector_partitioned();// Case 6
    test_fully_partitioned();        // Case 7
    
    test_default_case();             // Default case
    
    printf("\n=== All partition tests completed ===\n");
    
    // Additional variations to ensure compiler explores different paths
    printf("\n=== Additional variations for coverage ===\n");
    
    // Mixed data clauses
    {
        double a[N], b[N];
        init_array(a, N, 1.0);
        init_array(b, N, 2.0);
        
        #pragma acc parallel loop gang copy(a[0:N]) copyout(b[0:N])
        for (int i = 0; i < N; i++) {
            b[i] = a[i] * 3.0;
        }
        printf("Mixed copy/copyout test completed\n");
    }
    
    // Private variables
    {
        double shared = 10.0;
        #pragma acc parallel loop gang private(shared)
        for (int i = 0; i < N; i++) {
            double local = shared + i;
            // Use local to avoid optimization
            if (local < 0) local = 0;
        }
        printf("Private variable test completed\n");
    }
    
    // Runtime parameters
    {
        int dyn_size = N;
        double arr[N];
        init_array(arr, N, 1.5);
        
        #pragma acc parallel loop gang copy(arr[0:dyn_size])
        for (int i = 0; i < dyn_size; i++) {
            arr[i] = arr[i] * 2.0;
        }
        printf("Runtime parameter test completed\n");
    }
    
    // Reduction with array
    {
        double arr[N];
        double sum = 0.0;
        init_array(arr, N, 1.0);
        
        #pragma acc parallel loop reduction(+:sum) copy(arr[0:N])
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
        printf("Array reduction test completed: sum = %f\n", sum);
    }
    
    return 0;
}
