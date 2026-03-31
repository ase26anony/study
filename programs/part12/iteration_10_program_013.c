/* Test program to cover all partition code cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: partition code to string mapping
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define N 1024
#define M 64
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double val) {
    for (int i = 0; i < size; i++) {
        arr[i] = val + i * 0.1;
    }
}

/* Helper function to verify results */
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

/* Case 0: gang redundant - scalar reduction, no data partitioning across gangs */
void test_gang_redundant() {
    printf("Testing Case 0: gang redundant\n");
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

/* Case 1: gang partitioned - array data distributed across gangs */
void test_gang_partitioned() {
    printf("Testing Case 1: gang partitioned\n");
    double arr[N], result[N];
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

/* Case 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing Case 2: worker partitioned\n");
    double arr[M], result[M];
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

/* Case 3: gang+worker partitioned - nested gang and worker partitioning */
void test_gang_worker_partitioned() {
    printf("Testing Case 3: gang+worker partitioned\n");
    double arr[N][M], result[N][M];
    
    /* Initialize 2D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = 4.0 + i * 0.01 + j * 0.001;
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M]) copyout(result[0:N][0:M])
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                result[i][j] = arr[i][j] * 4.0;
            }
        }
    }
    
    /* Verify a sample */
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            double expected = (4.0 + i * 0.01 + j * 0.001) * 4.0;
            if (abs(result[i][j] - expected) > 1e-6) {
                errors++;
            }
        }
    }
    printf("  Errors in sample: %d\n", errors);
}

/* Case 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing Case 4: vector partitioned\n");
    double arr[N], result[N];
    init_array(arr, N, 5.0);
    
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

/* Case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing Case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    init_array(arr, N, 6.0);
    
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

/* Case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing Case 6: worker+vector partitioned\n");
    double arr[M], result[M];
    init_array(arr, M, 7.0);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 7.0;
        }
    }
    
    int errors = verify_array(result, M, 49.0);
    printf("  Errors: %d\n", errors);
}

/* Case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing Case 7: fully partitioned\n");
    double arr[N][M][P], result[N][M][P];
    
    /* Initialize 3D array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = 8.0 + i * 0.001 + j * 0.0001 + k * 0.00001;
            }
        }
    }
    
    #pragma acc parallel copyin(arr[0:N][0:M][0:P]) copyout(result[0:N][0:M][0:P])
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    result[i][j][k] = arr[i][j][k] * 8.0;
                }
            }
        }
    }
    
    /* Verify a small sample */
    int errors = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 5; k++) {
                double expected = (8.0 + i * 0.001 + j * 0.0001 + k * 0.00001) * 8.0;
                if (abs(result[i][j][k] - expected) > 1e-6) {
                    errors++;
                }
            }
        }
    }
    printf("  Errors in sample: %d\n", errors);
}

/* Test default case - This would normally be triggered through compiler internals
 * For testing purposes, we simulate the behavior */
void test_default_case() {
    printf("Testing default case (simulated)\n");
    printf("  Default case would be triggered by invalid partition codes\n");
    printf("  This requires compiler-internal testing hooks\n");
}

int main() {
    printf("Starting partition code coverage tests...\n\n");
    
    /* Execute all partition scenarios */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_default_case();
    
    printf("\nAll partition tests completed.\n");
    
    /* Additional test with runtime parameters to exercise dynamic partitioning */
    printf("\nTesting with runtime parameters...\n");
    {
        int dynamic_size = 512;
        double *dyn_arr = (double*)malloc(dynamic_size * sizeof(double));
        double *dyn_result = (double*)malloc(dynamic_size * sizeof(double));
        
        init_array(dyn_arr, dynamic_size, 10.0);
        
        /* Mixed partitioning with runtime bounds */
        #pragma acc parallel copyin(dyn_arr[0:dynamic_size]) copyout(dyn_result[0:dynamic_size])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < dynamic_size; i++) {
                dyn_result[i] = dyn_arr[i] * 2.0;
            }
        }
        
        free(dyn_arr);
        free(dyn_result);
    }
    
    printf("Runtime parameter test completed.\n");
    
    return 0;
}
