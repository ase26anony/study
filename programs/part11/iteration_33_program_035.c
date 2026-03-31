/* test_oacc_partition.c
 * 
 * This program exercises GCC's OpenACC partitioning logic to trigger
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32
#define P 16

/* Test 1: gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1)
 */
void test_gang_redundant(float *arr, int n) {
    float local_sum = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(n) reduction(+:local_sum) gang(1)
    {
        /* Simple gang-redundant computation */
        local_sum = 0.0f;
        for (int i = 0; i < n; i++) {
            local_sum += arr[i];
        }
    }
    
    arr[0] = local_sum / n;  /* Store average in first element */
}

/* Test 2: gang partitioned (case 1)
 * Outer loop explicitly marked as gang partitioned
 */
void test_gang_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = arr[i] * 2.0f + (float)i;
    }
}

/* Test 3: worker partitioned (case 2)
 * Inner loop marked as worker partitioned within nested loops
 */
void test_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M]) copyin(M)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = (float)(i + j) * 0.5f;
            }
        }
    }
}

/* Test 4: gang+worker partitioned (case 3)
 * Explicit gang and worker clauses on nested loops
 */
void test_gang_worker_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M]) copyin(M)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                arr[i][j] = arr[i][j] * 3.0f - (float)(i * j);
            }
        }
    }
}

/* Test 5: vector partitioned (case 4)
 * Loop marked for vector partitioning with element-wise operations
 */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        /* Element-wise operation suitable for vectorization */
        arr[i] = arr[i] * arr[i] - 2.0f * arr[i] + 1.0f;
    }
}

/* Test 6: gang+vector partitioned (case 5)
 * Combined gang and vector partitioning
 */
void test_gang_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop gang vector copy(arr[0:n]) copyin(n)
    for (int i = 0; i < n; i++) {
        arr[i] = 1.0f / (arr[i] + 1.0f);
    }
}

/* Test 7: worker+vector partitioned (case 6)
 * Worker and vector clauses on appropriate loop levels
 */
void test_worker_vector_partitioned(float arr[M][M]) {
    #pragma acc parallel copy(arr[0:M][0:M]) copyin(M)
    {
        #pragma acc loop gang
        for (int i = 0; i < M; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                arr[i][j] = (arr[i][j] + (float)(i - j)) * 0.25f;
            }
        }
    }
}

/* Test 8: fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, and vector clauses
 * Performing a stencil-like computation to create data dependencies
 */
void test_fully_partitioned(float arr3d[P][M][M]) {
    float temp[P][M][M];
    
    #pragma acc data copy(arr3d[0:P][0:M][0:M]) create(temp[0:P][0:M][0:M])
    {
        #pragma acc parallel copyin(P, M)
        {
            #pragma acc loop gang
            for (int k = 0; k < P; k++) {
                #pragma acc loop worker
                for (int i = 0; i < M; i++) {
                    #pragma acc loop vector
                    for (int j = 0; j < M; j++) {
                        /* Stencil computation with boundary checks */
                        float up = (i > 0) ? arr3d[k][i-1][j] : 0.0f;
                        float left = (j > 0) ? arr3d[k][i][j-1] : 0.0f;
                        float center = arr3d[k][i][j];
                        
                        temp[k][i][j] = (up + left + center) / 3.0f;
                    }
                }
            }
        }
        
        /* Copy back */
        #pragma acc parallel loop gang worker vector collapse(3) \
                copy(temp[0:P][0:M][0:M]) copyout(arr3d[0:P][0:M][0:M])
        for (int k = 0; k < P; k++) {
            for (int i = 0; i < M; i++) {
                for (int j = 0; j < M; j++) {
                    arr3d[k][i][j] = temp[k][i][j];
                }
            }
        }
    }
}

/* Main driver that conditionally executes test functions */
int main(int argc, char *argv[]) {
    /* Initialize test data */
    float arr1[N];
    float arr2[M][M];
    float arr3d[P][M][M];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i * 0.1f;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i * M + j) * 0.01f;
        }
    }
    
    for (int k = 0; k < P; k++) {
        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                arr3d[k][i][j] = (float)(k * M * M + i * M + j) * 0.001f;
            }
        }
    }
    
    /* Use argc to control which tests run, ensuring all code paths are compiled */
    int test_case = (argc > 1) ? atoi(argv[1]) : 0;
    
    /* Force compiler to analyze all OpenACC regions by wrapping in conditionals */
    if (test_case == 0 || test_case == 1) {
        test_gang_redundant(arr1, N);
    }
    
    if (test_case == 0 || test_case == 2) {
        test_gang_partitioned(arr1, N);
    }
    
    if (test_case == 0 || test_case == 3) {
        test_worker_partitioned(arr2);
    }
    
    if (test_case == 0 || test_case == 4) {
        test_gang_worker_partitioned(arr2);
    }
    
    if (test_case == 0 || test_case == 5) {
        test_vector_partitioned(arr1, N);
    }
    
    if (test_case == 0 || test_case == 6) {
        test_gang_vector_partitioned(arr1, N);
    }
    
    if (test_case == 0 || test_case == 7) {
        test_worker_vector_partitioned(arr2);
    }
    
    if (test_case == 0 || test_case == 8) {
        test_fully_partitioned(arr3d);
    }
    
    /* Print results to prevent dead code elimination */
    printf("Results:\n");
    printf("arr1[0] = %f, arr1[%d] = %f\n", arr1[0], N-1, arr1[N-1]);
    printf("arr2[0][0] = %f, arr2[%d][%d] = %f\n", 
           arr2[0][0], M-1, M-1, arr2[M-1][M-1]);
    printf("arr3d[0][0][0] = %f, arr3d[%d][%d][%d] = %f\n",
           arr3d[0][0][0], P-1, M-1, M-1, arr3d[P-1][M-1][M-1]);
    
    return 0;
}
