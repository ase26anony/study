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

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    
    double arr[M][M], result[M][M];
    init_array(&arr[0][0], M*M, 4.0);
    
    #pragma acc parallel copyin(arr[0:M][0:M]) copyout(result[0:M][0:M])
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

/* Test case 4: vector partitioned - vector-level SIMD operations */
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

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 6.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
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

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    
    double arr[N], result[N];
    init_array(arr, N, 7.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) num_workers(4) vector_length(16)
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

/* Test case 7: fully partitioned - all three levels (gang, worker, vector) */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    
    double arr[M][M][P], result[M][M][P];
    init_array(&arr[0][0][0], M*M*P, 8.0);
    
    #pragma acc parallel copyin(arr[0:M][0:M][0:P]) copyout(result[0:M][0:M][0:P])
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

/* Additional test with runtime parameters to influence partitioning */
void test_runtime_partitioning() {
    printf("Testing runtime partitioning decisions\n");
    
    int size = N;
    double *arr = (double*)malloc(size * sizeof(double));
    double *result = (double*)malloc(size * sizeof(double));
    
    init_array(arr, size, 9.0);
    
    // Use runtime variable for vector length
    int vec_len = 32;
    
    #pragma acc parallel copyin(arr[0:size]) copyout(result[0:size]) vector_length(vec_len)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < size; i++) {
            result[i] = arr[i] * 9.0;
        }
    }
    
    // Verify
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (result[i] != arr[i] * 9.0) errors++;
    }
    printf("  Errors: %d\n", errors);
    
    free(arr);
    free(result);
}

/* Test with different data clauses to influence partitioning */
void test_data_clauses() {
    printf("Testing various data clauses\n");
    
    double arr1[N], arr2[N], arr3[N];
    double result1[N], result2[N], result3[N];
    
    init_array(arr1, N, 10.0);
    init_array(arr2, N, 11.0);
    init_array(arr3, N, 12.0);
    
    // Mixed data clauses
    #pragma acc data copyin(arr1[0:N], arr2[0:N]) create(result1[0:N], result2[0:N]) copyout(result3[0:N])
    {
        #pragma acc parallel present(arr1, arr2, result1, result2, result3)
        {
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                result1[i] = arr1[i] * 10.0;
                result2[i] = arr2[i] * 11.0;
                result3[i] = result1[i] + result2[i];
            }
        }
    }
    
    printf("  Data clause test completed\n");
}

/* Test reduction operations with different partitioning */
void test_reductions() {
    printf("Testing reduction operations\n");
    
    double arr[N];
    init_array(arr, N, 1.5);
    
    // Scalar reduction - typically gang redundant
    double sum = 0.0;
    #pragma acc parallel copyin(arr[0:N]) reduction(+:sum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            sum += arr[i];
        }
    }
    
    // Array reduction - may use different partitioning
    double max_val = arr[0];
    #pragma acc parallel copyin(arr[0:N]) reduction(max:max_val)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            if (arr[i] > max_val) max_val = arr[i];
        }
    }
    
    printf("  Sum: %f, Max: %f\n", sum, max_val);
}

int main() {
    printf("Starting partition coverage tests...\n\n");
    
    /* Execute all test cases to trigger different partition codes */
    test_gang_redundant();           /* Should trigger case 0 */
    test_gang_partitioned();         /* Should trigger case 1 */
    test_worker_partitioned();       /* Should trigger case 2 */
    test_gang_worker_partitioned();  /* Should trigger case 3 */
    test_vector_partitioned();       /* Should trigger case 4 */
    test_gang_vector_partitioned();  /* Should trigger case 5 */
    test_worker_vector_partitioned();/* Should trigger case 6 */
    test_fully_partitioned();        /* Should trigger case 7 */
    
    /* Additional tests for compiler analysis */
    test_runtime_partitioning();
    test_data_clauses();
    test_reductions();
    
    printf("\nAll tests completed.\n");
    printf("Note: To trigger the default case (<illegal>), the compiler's internal\n");
    printf("testing interfaces would need to be used with invalid partition codes.\n");
    
    return 0;
}
