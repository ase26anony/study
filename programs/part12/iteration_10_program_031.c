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

/* Helper to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Test case 0: gang redundant - scalar reductions, no data partitioning across gangs */
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

/* Test case 1: gang partitioned - array data distributed across gangs only */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N];
    init_array(arr, N, 1.0);
    
    #pragma acc parallel loop gang copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * 2.0;
    }
    
    // Verify
    double check = 0.0;
    for (int i = 0; i < N; i++) {
        check += arr[i];
    }
    printf("  Result: array sum = %f\n", check);
}

/* Test case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    double arr[M];
    init_array(arr, M, 2.0);
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = arr[i] + i * 0.01;
    }
    
    double check = 0.0;
    for (int i = 0; i < M; i++) {
        check += arr[i];
    }
    printf("  Result: array sum = %f\n", check);
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M];
    
    // Initialize
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = i * M + j;
        }
    }
    
    #pragma acc parallel loop gang worker collapse(2) copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = arr[i][j] * 1.5;
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

/* Test case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    float arr[N];
    for (int i = 0; i < N; i++) {
        arr[i] = i * 0.25f;
    }
    
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * 2.0f;
    }
    
    float check = 0.0f;
    for (int i = 0; i < N; i++) {
        check += arr[i];
    }
    printf("  Result: float array sum = %f\n", check);
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N];
    init_array(arr, N, 3.0);
    
    #pragma acc parallel loop gang vector copy(arr[0:N])
    for (int i = 0; i < N; i++) {
        arr[i] = arr[i] * 3.0;
    }
    
    double check = 0.0;
    for (int i = 0; i < N; i++) {
        check += arr[i];
    }
    printf("  Result: array sum = %f\n", check);
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    int arr[M];
    for (int i = 0; i < M; i++) {
        arr[i] = i;
    }
    
    #pragma acc parallel loop worker vector num_workers(2) vector_length(16) copy(arr[0:M])
    for (int i = 0; i < M; i++) {
        arr[i] = arr[i] * 2;
    }
    
    int check = 0;
    for (int i = 0; i < M; i++) {
        check += arr[i];
    }
    printf("  Result: int array sum = %d\n", check);
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P];
    
    // Initialize 3D array
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = i * M * P + j * P + k;
            }
        }
    }
    
    #pragma acc parallel loop gang worker vector collapse(3) copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = arr[i][j][k] * 1.1;
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

/* Additional tests with different data clauses and patterns */
void test_variations() {
    printf("\nTesting additional variations:\n");
    
    /* Variation 1: present clause with gang partitioning */
    double present_arr[N];
    init_array(present_arr, N, 5.0);
    
    #pragma acc enter data copyin(present_arr[0:N])
    
    #pragma acc parallel loop gang present(present_arr[0:N])
    for (int i = 0; i < N; i++) {
        present_arr[i] = present_arr[i] / 2.0;
    }
    
    #pragma acc exit data copyout(present_arr[0:N])
    
    /* Variation 2: private variables with worker partitioning */
    int private_var;
    #pragma acc parallel loop worker private(private_var) num_workers(4)
    for (int i = 0; i < M; i++) {
        private_var = i * 2;
        // Use private_var
    }
    
    /* Variation 3: reduction with array elements (partial gang partitioning) */
    double partial_reduce = 0.0;
    double arr2[N];
    init_array(arr2, N, 1.5);
    
    #pragma acc parallel loop gang reduction(+:partial_reduce) copy(arr2[0:N])
    for (int i = 0; i < N; i++) {
        partial_reduce += arr2[i];
        arr2[i] = arr2[i] * 0.5;
    }
    
    printf("  Variations completed\n");
}

/* Test edge cases and runtime parameters */
void test_edge_cases() {
    printf("\nTesting edge cases with runtime parameters:\n");
    
    int runtime_n = 512;
    double *dynamic_arr = (double*)malloc(runtime_n * sizeof(double));
    init_array(dynamic_arr, runtime_n, 2.0);
    
    /* Runtime gang count */
    int num_gangs = 4;
    #pragma acc parallel loop gang num_gangs(num_gangs) copy(dynamic_arr[0:runtime_n])
    for (int i = 0; i < runtime_n; i++) {
        dynamic_arr[i] = dynamic_arr[i] + 1.0;
    }
    
    /* Triangular loop (non-rectangular) */
    int sum = 0;
    #pragma acc parallel loop reduction(+:sum)
    for (int i = 0; i < runtime_n; i++) {
        for (int j = 0; j < i; j++) {
            sum += 1;
        }
    }
    
    free(dynamic_arr);
    printf("  Edge cases completed, triangular sum = %d\n", sum);
}

int main() {
    printf("Starting partition coverage tests...\n\n");
    
    /* Execute all 8 partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Additional tests for coverage */
    test_variations();
    test_edge_cases();
    
    printf("\nAll partition tests completed successfully!\n");
    
    /* Note: The default case (case 8+) would require compiler-internal
     * testing hooks or invalid partition codes, which cannot be
     * directly triggered from user code. This would need separate
     * compiler testing infrastructure.
     */
    
    return 0;
}
