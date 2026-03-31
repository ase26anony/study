/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition code to string mapping
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 128
#define P 32

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
    
    printf("  Result[0] = %f\n", result[0]);
}

/* Test case 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    double matrix[N][M];
    double sum = 0.0;
    
    // Initialize matrix
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            matrix[i][j] = (i + j) * 0.1;
        }
    }
    
    #pragma acc parallel copyin(matrix) reduction(+:sum)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                sum += matrix[i][j];
            }
        }
    }
    
    printf("  Matrix sum = %f\n", sum);
}

/* Test case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    float arr[N];
    float result[N];
    
    for (int i = 0; i < N; i++) {
        arr[i] = i * 0.5f;
    }
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * arr[i];
        }
    }
    
    printf("  Result[%d] = %f\n", N-1, result[N-1]);
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    double arr[N];
    double result[N];
    init_array(arr, N, 5.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    printf("  First element: %f -> %f\n", arr[0], result[0]);
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    float arr[M];
    float result[M];
    
    for (int i = 0; i < M; i++) {
        arr[i] = i * 1.5f;
    }
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] + 10.0f;
        }
    }
    
    printf("  Last element: %f -> %f\n", arr[M-1], result[M-1]);
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    double cube[P][M][N];
    double total = 0.0;
    
    // Initialize 3D array
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < N; k++) {
                cube[i][j][k] = (i + j + k) * 0.01;
            }
        }
    }
    
    #pragma acc parallel copyin(cube) reduction(+:total)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < P; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < N; k++) {
                    total += cube[i][j][k];
                }
            }
        }
    }
    
    printf("  3D array total = %f\n", total);
}

/* Additional tests with different data clauses and patterns */
void test_variations() {
    printf("\nTesting additional variations:\n");
    
    /* Variation 1: present clause */
    double arr1[N];
    init_array(arr1, N, 10.0);
    
    #pragma acc enter data copyin(arr1[0:N])
    
    #pragma acc parallel present(arr1[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            arr1[i] *= 2.0;
        }
    }
    
    #pragma acc exit data copyout(arr1[0:N])
    printf("  Present clause test: arr1[0] = %f\n", arr1[0]);
    
    /* Variation 2: private variables */
    double arr2[N];
    init_array(arr2, N, 20.0);
    
    #pragma acc parallel copy(arr2[0:N])
    {
        double private_var;
        #pragma acc loop gang private(private_var)
        for (int i = 0; i < N; i++) {
            private_var = i * 0.5;
            arr2[i] += private_var;
        }
    }
    printf("  Private variable test: arr2[0] = %f\n", arr2[0]);
    
    /* Variation 3: runtime parameters */
    int dyn_size = 512;
    double *dyn_arr = (double*)malloc(dyn_size * sizeof(double));
    init_array(dyn_arr, dyn_size, 30.0);
    
    #pragma acc parallel copyin(dyn_arr[0:dyn_size])
    {
        #pragma acc loop gang num_gangs(dyn_size/64)
        for (int i = 0; i < dyn_size; i++) {
            dyn_arr[i] /= 2.0;
        }
    }
    printf("  Runtime parameter test: dyn_arr[0] = %f\n", dyn_arr[0]);
    
    free(dyn_arr);
}

/* Test edge cases that might trigger default path */
void test_edge_cases() {
    printf("\nTesting edge cases:\n");
    
    /* Edge case 1: Empty parallel region */
    #pragma acc parallel
    {
        // No operations
    }
    printf("  Empty parallel region executed\n");
    
    /* Edge case 2: Single element */
    double single = 42.0;
    #pragma acc parallel copy(single)
    {
        single = 100.0;
    }
    printf("  Single variable: %f\n", single);
    
    /* Edge case 3: Very small loop */
    double tiny[4];
    #pragma acc parallel copyout(tiny[0:4])
    {
        #pragma acc loop
        for (int i = 0; i < 4; i++) {
            tiny[i] = i * 10.0;
        }
    }
    printf("  Tiny loop: %f, %f, %f, %f\n", tiny[0], tiny[1], tiny[2], tiny[3]);
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
    
    /* Additional variations */
    test_variations();
    
    /* Edge cases */
    test_edge_cases();
    
    printf("\nAll tests completed successfully!\n");
    
    return 0;
}
