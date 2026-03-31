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
#define P 32

/* Helper function to initialize arrays */
void init_array(double *arr, int size, double value) {
    for (int i = 0; i < size; i++) {
        arr[i] = value + i * 0.1;
    }
}

/* Test 0: gang redundant - scalar reductions, no data partitioning across gangs */
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

/* Test 1: gang partitioned - array data distributed across gangs */
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
        if (abs(result[i] - (2.0 + i * 0.1) * 2.0) > 0.0001) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test 2: worker partitioned - worker-level distribution */
void test_worker_partitioned() {
    printf("Testing case 2: worker partitioned\n");
    
    double arr[M], result[M];
    init_array(arr, M, 3.0);
    
    #pragma acc parallel loop worker num_workers(4) copy(arr[0:M]) copyout(result[0:M])
    for (int i = 0; i < M; i++) {
        result[i] = arr[i] * 3.0;
    }
    
    printf("  Result[0] = %f\n", result[0]);
}

/* Test 3: gang+worker partitioned - nested gang and worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    double arr[N][M];
    double sum = 0.0;
    
    // Initialize 2D array
    #pragma acc parallel loop collapse(2) copyout(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            arr[i][j] = (i * M + j) * 0.01;
        }
    }
    
    #pragma acc parallel loop gang worker collapse(2) reduction(+:sum) copy(arr[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            sum += arr[i][j];
        }
    }
    
    printf("  2D sum = %f\n", sum);
}

/* Test 4: vector partitioned - vector-level SIMD operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 4.0);
    
    #pragma acc parallel loop vector vector_length(32) copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = arr[i] * arr[i];
    }
    
    printf("  Vector result[10] = %f\n", result[10]);
}

/* Test 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 5.0);
    
    #pragma acc parallel loop gang vector copy(arr[0:N]) copyout(result[0:N])
    for (int i = 0; i < N; i++) {
        result[i] = sin(arr[i]) + cos(arr[i]);
    }
    
    printf("  Gang+vector result[20] = %f\n", result[20]);
}

/* Test 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    double arr[M], result[M];
    init_array(arr, M, 6.0);
    
    #pragma acc parallel loop worker vector num_workers(4) vector_length(16) \
                copy(arr[0:M]) copyout(result[0:M])
    for (int i = 0; i < M; i++) {
        result[i] = exp(arr[i] * 0.01);
    }
    
    printf("  Worker+vector result[5] = %f\n", result[5]);
}

/* Test 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    double arr[N][M][P];
    double total = 0.0;
    
    // Initialize 3D array
    #pragma acc parallel loop collapse(3) copyout(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                arr[i][j][k] = (i * M * P + j * P + k) * 0.001;
            }
        }
    }
    
    #pragma acc parallel loop gang worker vector collapse(3) reduction(+:total) \
                copy(arr[0:N][0:M][0:P])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                total += arr[i][j][k];
            }
        }
    }
    
    printf("  3D total = %f\n", total);
}

/* Test default case - if compiler testing interfaces were available */
void test_default_case() {
    printf("Testing default case (simulated)\n");
    printf("  Note: Actual default case would require invalid partition code\n");
    printf("  This might be triggered via compiler internals or malformed directives\n");
    
    /* Example of potentially problematic directive that might lead to unexpected codes */
    volatile int dynamic_size = N;
    double *dynamic_arr = (double*)malloc(dynamic_size * sizeof(double));
    
    if (dynamic_arr) {
        init_array(dynamic_arr, dynamic_size, 7.0);
        
        /* This might produce unusual partitioning in some edge cases */
        #pragma acc parallel loop copy(dynamic_arr[0:dynamic_size])
        for (int i = 0; i < dynamic_size; i++) {
            dynamic_arr[i] *= 2.0;
        }
        
        free(dynamic_arr);
    }
}

int main() {
    printf("Starting partition coverage tests...\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    
    /* Test for default case coverage */
    test_default_case();
    
    printf("\nAll partition tests completed.\n");
    
    /* Additional test with runtime parameters to exercise dynamic partitioning */
    printf("\nTesting with runtime parameters:\n");
    int runtime_n = 512;
    double runtime_arr[512];
    init_array(runtime_arr, runtime_n, 8.0);
    
    #pragma acc parallel loop gang num_gangs(runtime_n/64) copy(runtime_arr[0:runtime_n])
    for (int i = 0; i < runtime_n; i++) {
        runtime_arr[i] = runtime_arr[i] / 2.0;
    }
    
    printf("Runtime parameter test completed.\n");
    
    return 0;
}
