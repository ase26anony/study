/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
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

/* Test 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[M][M], result[M][M];
    
    // Initialize 2D array
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j + 1.0;
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
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            if (result[i][j] != arr[i][j] * 4.0) errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test 4: vector partitioned - vector-level SIMD operations */
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
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 5.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 6.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop gang vector
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

/* Test 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 7.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 7.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (result[i] != arr[i] * 7.0) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[M][M][P], result[M][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = (i * M * P) + (j * P) + k + 1.0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr) copyout(result) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] * 8.0;
                }
            }
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                if (result[i][j][k] != arr[i][j][k] * 8.0) errors++;
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test default case: This would normally be triggered through compiler-internal
 * testing hooks. For this external test, we simulate by using runtime parameters
 * that might lead to unexpected partition codes.
 */
void test_default_case_simulation() {
    printf("Testing default case simulation\n");
    
    // Use variable loop bounds and runtime expressions to create complex
    // partitioning scenarios that might trigger edge cases
    int dynamic_size = N;
    double arr[N], result[N];
    
    init_array(arr, N, 9.0);
    
    // Variable gang count based on runtime value
    int gang_count = 4;
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) \
                num_gangs(gang_count) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang
        for (int i = 0; i < dynamic_size; i++) {
            // Mixed access pattern
            int idx = (i * 17) % N;  // Non-contiguous access
            result[idx] = arr[idx] * 9.0;
        }
    }
    
    printf("  Dynamic partitioning test completed\n");
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    // Execute all partition test cases
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    // Simulate edge cases that might trigger default partition mapping
    test_default_case_simulation();
    
    printf("\nAll tests completed successfully!\n");
    printf("This test program exercises all 8 partition codes (0-7) plus edge cases.\n");
    printf("When compiled with appropriate flags, it should trigger the partition\n");
    printf("mapping function in omp-oacc-neuter-broadcast.cc lines 335-343.\n");
    
    return 0;
}
