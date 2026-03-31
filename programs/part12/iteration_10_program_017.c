/* Test program to cover all partition mapping cases in omp-oacc-neuter-broadcast.cc
 * Lines 335-343: case 0-7 partition string mappings
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o partition_test partition_test.c
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

/* Test case 0: gang redundant - scalar reductions, no data partitioning */
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

/* Test case 1: gang partitioned - array distributed across gangs */
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
        if (abs(result[i] - (2.0 + i*0.1)*2.0) > 0.0001) errors++;
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
    
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (abs(result[i] - (3.0 + i*0.1)*3.0) > 0.0001) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test case 3: gang+worker partitioned - nested gang/worker distribution */
void test_gang_worker_partitioned() {
    printf("Testing case 3: gang+worker partitioned\n");
    double arr[N][M], result[N][M];
    
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
            if (abs(result[i][j] - expected) > 0.0001) errors++;
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test case 4: vector partitioned - SIMD-style vector operations */
void test_vector_partitioned() {
    printf("Testing case 4: vector partitioned\n");
    double arr[N], result[N];
    init_array(arr, N, 4.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N]) vector_length(32)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 4.0;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (abs(result[i] - (4.0 + i*0.1)*4.0) > 0.0001) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test case 5: gang+vector partitioned - gang and vector without workers */
void test_gang_vector_partitioned() {
    printf("Testing case 5: gang+vector partitioned\n");
    double arr[N], result[N];
    init_array(arr, N, 5.0);
    
    #pragma acc parallel copyin(arr[0:N]) copyout(result[0:N])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            result[i] = arr[i] * 5.0;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (abs(result[i] - (5.0 + i*0.1)*5.0) > 0.0001) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test case 6: worker+vector partitioned - worker and vector combination */
void test_worker_vector_partitioned() {
    printf("Testing case 6: worker+vector partitioned\n");
    double arr[M], result[M];
    init_array(arr, M, 6.0);
    
    #pragma acc parallel copyin(arr[0:M]) copyout(result[0:M]) num_workers(4) vector_length(16)
    {
        #pragma acc loop worker vector
        for (int i = 0; i < M; i++) {
            result[i] = arr[i] * 6.0;
        }
    }
    
    int errors = 0;
    for (int i = 0; i < M; i++) {
        if (abs(result[i] - (6.0 + i*0.1)*6.0) > 0.0001) errors++;
    }
    printf("  Errors: %d\n", errors);
}

/* Test case 7: fully partitioned - gang, worker, and vector all active */
void test_fully_partitioned() {
    printf("Testing case 7: fully partitioned\n");
    double arr[N][M][P], result[N][M][P];
    
    #pragma acc parallel copyin(arr) copyout(result)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < P; k++) {
                    arr[i][j][k] = i * 10000.0 + j * 100.0 + k;
                    result[i][j][k] = arr[i][j][k] * 7.0;
                }
            }
        }
    }
    
    int errors = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            for (int k = 0; k < P; k++) {
                double expected = (i * 10000.0 + j * 100.0 + k) * 7.0;
                if (abs(result[i][j][k] - expected) > 0.0001) errors++;
            }
        }
    }
    printf("  Errors: %d\n", errors);
}

/* Test default case: This would require compiler internals or error injection.
   For coverage, we include a malformed directive that might trigger unexpected values. */
void test_default_case() {
    printf("Testing potential default case (if compiler supports)\n");
    
    /* Try to create ambiguous partitioning scenario */
    double arr[10];
    #pragma acc parallel copy(arr[0:10])
    {
        /* Empty parallel region - might produce unexpected partition code */
        #pragma acc loop
        for (int i = 0; i < 10; i++) {
            arr[i] = i;
        }
    }
}

int main() {
    printf("Starting partition mapping coverage tests...\n\n");
    
    /* Execute all partition test cases */
    test_gang_redundant();           /* Case 0 */
    test_gang_partitioned();         /* Case 1 */
    test_worker_partitioned();       /* Case 2 */
    test_gang_worker_partitioned();  /* Case 3 */
    test_vector_partitioned();       /* Case 4 */
    test_gang_vector_partitioned();  /* Case 5 */
    test_worker_vector_partitioned();/* Case 6 */
    test_fully_partitioned();        /* Case 7 */
    
    /* Optional: test default case */
    test_default_case();
    
    printf("\nAll partition tests completed.\n");
    printf("To verify coverage, compile with:\n");
    printf("  gcc -O1 -fopenacc -fdump-tree-omplower -g -fstack-protector-strong \\\n");
    printf("      -o partition_test partition_test.c\n");
    
    return 0;
}
