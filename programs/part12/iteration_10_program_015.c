/* Test program to exercise all OpenACC partition mapping cases
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 * Or with OpenMP: gcc -O3 -fopenmp -fopenmp-targets=nvptx64-nvidia-cuda -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 16

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double start_val) {
    for (int i = 0; i < size; i++) {
        arr[i] = start_val + i * 0.1;
    }
}

/* Helper function to verify results */
int verify_array(double *arr, int size, double expected_base) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        double expected = expected_base + i * 0.1;
        if (abs(arr[i] - expected) > 1e-6) {
            errors++;
            if (errors < 5) {
                printf("  Error at index %d: got %f, expected %f\n", 
                       i, arr[i], expected);
            }
        }
    }
    return errors;
}

/* Test Case 0: gang redundant - scalar reductions, no data partitioning */
void test_gang_redundant() {
    printf("Testing case 0: gang redundant\n");
    
    double sum = 0.0;
    double arr[N];
    init_array(arr, N, 1.0);
    
    #pragma acc parallel copy(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    printf("  Sum: %f (expected ~%f)\n", sum, N * 1.0 + (N-1)*N*0.1/2);
}

/* Test Case 1: gang partitioned - array distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    
    double arr[N];
    double result[N];
    init_array(arr, N, 2.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 2.0;
        }
    }
    
    int errors = verify_array(result, N, 4.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    
    double arr[M];
    double result[M];
    init_array(arr, M, 3.0);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 3.0;
        }
    }
    
    int errors = verify_array(result, M, 9.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    double arr[N][M];
    double result[N][M];
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                arr[i][j] = i * 100.0 + j;
                result[i][j] = arr[i][j] * 2.0;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            double expected = (i * 100.0 + j) * 2.0;
            if (abs(result[i][j] - expected) > 1e-6) {
                errors++;
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test Case 4: vector partitioned - SIMD-style vector operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    double arr[N];
    double result[N];
    init_array(arr, N, 4.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 4.0;
        }
    }
    
    int errors = verify_array(result, N, 16.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
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
    
    int errors = verify_array(result, N, 25.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    double arr[M];
    double result[M];
    init_array(arr, M, 6.0);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    int errors = verify_array(result, M, 36.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    double arr[N][M][P];
    double result[N][M][P];
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = i * 10000.0 + j * 100.0 + k;
                    result[i][j][k] = arr[i][j][k] * 2.0;
                }
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                double expected = (i * 10000.0 + j * 100.0 + k) * 2.0;
                if (abs(result[i][j][k] - expected) > 1e-6) {
                    errors++;
                }
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test default case through edge conditions */
void test_edge_cases() {
    printf("Testing edge cases (may trigger default partition mapping)\n");
    
    /* Test with runtime-variable loop bounds */
    int dynamic_size = N;
    double *dynamic_arr = (double*)malloc(dynamic_size * sizeof(double));
    double *dynamic_result = (double*)malloc(dynamic_size * sizeof(double));
    
    init_array(dynamic_arr, dynamic_size, 7.0);
    
    #pragma acc parallel copyin(dynamic_arr[0:dynamic_size]) \
                         copyout(dynamic_result[0:dynamic_size])
    {
        #pragma acc loop gang
        for (int i = 0; i < dynamic_size; i++) {
            dynamic_result[i] = dynamic_arr[i] * 7.0;
        }
    }
    
    int errors = verify_array(dynamic_result, dynamic_size, 49.0);
    printf("  Dynamic array errors: %d\n", errors);
    
    free(dynamic_arr);
    free(dynamic_result);
    
    /* Test triangular loop (non-rectangular iteration space) */
    double triangular[N];
    #pragma acc parallel copyout(triangular[0:N])
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker
            for (int j = 0; j <= i; j++) {
                triangular[i] = i * 1.0;
            }
        }
    }
    
    /* Test with private variables */
    double private_test[N];
    init_array(private_test, N, 8.0);
    
    #pragma acc parallel copy(private_test[0:N])
    {
        #pragma acc loop gang private(private_test)
        for (int i = 0; i < N; i++) {
            double local_var = private_test[i];
            private_test[i] = local_var * 2.0;
        }
    }
    
    errors = verify_array(private_test, N, 16.0);
    printf("  Private variable errors: %d\n", errors);
}

int main() {
    printf("Starting OpenACC partition mapping tests...\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    printf("\n");
    
    test_gang_partitioned();
    printf("\n");
    
    test_worker_partitioned();
    printf("\n");
    
    test_gang_worker_partitioned();
    printf("\n");
    
    test_vector_partitioned();
    printf("\n");
    
    test_gang_vector_partitioned();
    printf("\n");
    
    test_worker_vector_partitioned();
    printf("\n");
    
    test_fully_partitioned();
    printf("\n");
    
    test_edge_cases();
    printf("\n");
    
    printf("All tests completed.\n");
    
    return 0;
}
