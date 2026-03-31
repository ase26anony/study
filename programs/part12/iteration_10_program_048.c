/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: Partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
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

/* Helper to verify results */
int verify_array(double *arr, int size, double expected_base) {
    int errors = 0;
    for (int i = 0; i < size; i++) {
        double expected = expected_base + i * 0.1;
        if (abs(arr[i] - expected) > 1e-6) {
            errors++;
        }
    }
    return errors;
}

/* Test Case 0: gang redundant - scalar reductions, no data partitioning across gangs */
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

/* Test Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing case 1: gang partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 2.0);
    memset(result, 0, N * sizeof(double));
    
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
    double arr[N], result[N];
    
    init_array(arr, N, 3.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4)
    {
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 3.0;
        }
    }
    
    int errors = verify_array(result, N, 9.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[M][P], result[M][P];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            arr[i][j] = 4.0 + i * 0.01 + j * 0.001;
            result[i][j] = 0.0;
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:P]) copyout(result[0:M][0:P])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < P; j++) {
                result[i][j] = arr[i][j] * 4.0;
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            double expected = (4.0 + i * 0.01 + j * 0.001) * 4.0;
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
    double arr[N], result[N];
    
    init_array(arr, N, 5.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    int errors = verify_array(result, N, 25.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 6.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    int errors = verify_array(result, N, 36.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[N], result[N];
    
    init_array(arr, N, 7.0);
    memset(result, 0, N * sizeof(double));
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 7.0;
        }
    }
    
    int errors = verify_array(result, N, 49.0);
    printf("  Errors: %d\n", errors);
}

/* Test Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[M][P][8], result[M][P][8];
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 8; k++) {
                arr[i][j][k] = 8.0 + i * 0.01 + j * 0.001 + k * 0.0001;
                result[i][j][k] = 0.0;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:M][0:P][0:8]) copyout(result[0:M][0:P][0:8])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < P; j++) {
                for (int k = 0; k < 8; k++) {
                    result[i][j][k] = arr[i][j][k] * 8.0;
                }
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            for (int k = 0; k < 8; k++) {
                double expected = (8.0 + i * 0.01 + j * 0.001 + k * 0.0001) * 8.0;
                if (abs(result[i][j][k] - expected) > 1e-6) {
                    errors++;
                }
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test default case - This would normally be triggered through compiler internals
 * For testing purposes, we include a malformed directive that might produce unexpected values
 */
void test_default_case() {
    printf("Testing potential default case triggers\n");
    
    /* Using variable loop bounds that might confuse partitioning analysis */
    int dynamic_size = N;
    double arr[N], result[N];
    
    init_array(arr, N, 9.0);
    memset(result, 0, N * sizeof(double));
    
    /* Complex directive with multiple clauses that might produce edge cases */
    #pragma acc parallel copyin(arr[0:dynamic_size]) copyout(result[0:dynamic_size])
    {
        #pragma acc loop independent
        for (int i = 0; i < dynamic_size; i++) {
            result[i] = arr[i] * 9.0;
        }
    }
    
    int errors = verify_array(result, N, 81.0);
    printf("  Errors: %d\n", errors);
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all test cases to trigger different partition codes */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    test_default_case();             /* May trigger default case or edge conditions */
    
    printf("\nAll tests completed.\n");
    
    /* Additional test with runtime parameters to exercise dynamic partitioning */
    printf("\nTesting with runtime parameters...\n");
    int runtime_n = 512;
    double *runtime_arr = (double*)malloc(runtime_n * sizeof(double));
    double *runtime_res = (double*)malloc(runtime_n * sizeof(double));
    
    init_array(runtime_arr, runtime_n, 10.0);
    
    /* Mixed partitioning with runtime expressions */
    #pragma acc parallel copyin(runtime_arr[0:runtime_n]) copyout(runtime_res[0:runtime_n]) \
        num_gangs(runtime_n/64) num_workers(4) vector_length(16)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < runtime_n; i++) {
            runtime_res[i] = runtime_arr[i] * 10.0;
        }
    }
    
    int runtime_errors = verify_array(runtime_res, runtime_n, 100.0);
    printf("Runtime test errors: %d\n", runtime_errors);
    
    free(runtime_arr);
    free(runtime_res);
    
    return 0;
}
