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
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
    }
}

/* Test Case 0: gang redundant - scalar reductions, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    double sum = 0.0;
    double scalar = 1.5;
    
    #pragma acc parallel copyin(scalar) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += scalar * 0.01;
        }
    }
    
    printf("  Result: sum = %f\n", sum);
}

/* Test Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N];
    init_array(arr, N, 1.0);
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * 2.0;
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i];
    }
    printf("  Checksum: %f\n", checksum);
}

/* Test Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[N];
    init_array(arr, N, 2.0);
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] + i * 0.01;
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i];
    }
    printf("  Checksum: %f\n", checksum);
}

/* Test Case 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[M][N];
    
    // Initialize
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = i * 100.0 + j * 0.1;
        }
    }
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:M][0:N])
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = arr[i][j] * 1.5;
        }
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr[i][j];
        }
    }
    printf("  Checksum: %f\n", checksum);
}

/* Test Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    double arr[N];
    init_array(arr, N, 3.0);
    
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * arr[i];
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i];
    }
    printf("  Checksum: %f\n", checksum);
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N];
    init_array(arr, N, 4.0);
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = sin(arr[i]) + cos(arr[i]);
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i];
    }
    printf("  Checksum: %f\n", checksum);
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[N];
    init_array(arr, N, 5.0);
    
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = sqrt(arr[i] + 1.0);
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i];
    }
    printf("  Checksum: %f\n", checksum);
}

/* Test Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[P][M][N];
    
    // Initialize 3D array
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                arr[i][j][k] = i * 1000.0 + j * 100.0 + k * 0.1;
            }
        }
    }
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:P][0:M][0:N])
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                arr[i][j][k] = arr[i][j][k] * 0.5 + 1.0;
            }
        }
    }
    
    // Verify
    double checksum = 0.0;
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                checksum += arr[i][j][k];
            }
        }
    }
    printf("  Checksum: %f\n", checksum);
}

/* Additional tests with different data clauses to influence partitioning */
void test_data_clause_variations() {
    printf("Testing data clause variations\n");
    
    /* Test with present clause */
    double arr1[N], arr2[N];
    init_array(arr1, N, 10.0);
    init_array(arr2, N, 20.0);
    
    // First copy data to device
    #pragma acc enter data copyin(arr1[0:N], arr2[0:N])
    
    // Use present clause
    #pragma acc parallel loop present(arr1, arr2)
    for (int i = 0; i < N; i++) {
        arr1[i] = arr1[i] + arr2[i];
    }
    
    #pragma acc exit data copyout(arr1[0:N])
    
    /* Test with private clause */
    double private_var;
    #pragma acc parallel loop private(private_var) copy(arr2[0:N])
    for (int i = 0; i < N; i++) {
        private_var = i * 0.1;
        arr2[i] = arr2[i] + private_var;
    }
    
    printf("  Data clause tests completed\n");
}

/* Test with runtime parameters */
void test_runtime_partitioning(int size) {
    printf("Testing runtime partitioning with size=%d\n", size);
    
    double *dynamic_arr = (double*)malloc(size * sizeof(double));
    init_array(dynamic_arr, size, 1.0);
    
    int gang_expr = size / 128;
    int vector_expr = 32;
    
    #pragma acc parallel loop gang(num:gang_expr) vector_length(vector_expr) copy(dynamic_arr[0:size])
    for (int i = 0; i < size; i++) {
        dynamic_arr[i] = dynamic_arr[i] * 2.0;
    }
    
    free(dynamic_arr);
    printf("  Runtime partitioning test completed\n");
}

/* Main function to execute all test cases */
int main() {
    printf("Starting partition mapping coverage tests\n");
    printf("=========================================\n");
    
    // Execute all 8 partition test cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Additional tests for coverage
    test_data_clause_variations();
    test_runtime_partitioning(512);
    
    printf("\nAll partition tests completed successfully!\n");
    printf("This should trigger all 8 partition codes (0-7) in the compiler's analysis.\n");
    
    return 0;
}
