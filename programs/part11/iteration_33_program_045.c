/* test_oacc_partition.c
 * 
 * This program exercises OpenACC partitioning logic to trigger
 * the switch cases in omp-oacc-neuter-broadcast.cc lines 335-343.
 * Each test function targets a specific partitioning pattern.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 32

/* Test 1: Gang redundant (case 0)
 * A parallel region without an associated loop, or with gang(1)
 */
void test_gang_redundant(float *arr, int n) {
    float scalar = 0.0f;
    
    #pragma acc parallel copy(arr[0:n]) copyin(n) gang(1)
    {
        /* Simple assignment - gang redundant */
        arr[0] = 1.0f;
        scalar = 2.0f;
        arr[n-1] = scalar;
    }
    
    printf("Gang redundant: arr[0]=%.2f, arr[%d]=%.2f\n", arr[0], n-1, arr[n-1]);
}

/* Test 2: Gang partitioned (case 1)
 * Loop with explicit gang partitioning
 */
void test_gang_partitioned(float *arr, int n) {
    float sum = 0.0f;
    
    #pragma acc parallel loop gang copy(arr[0:n]) reduction(+:sum)
    for (int i = 0; i < n; i++) {
        arr[i] = (float)i * 0.5f;
        sum += arr[i];
    }
    
    printf("Gang partitioned: sum=%.2f, arr[100]=%.2f\n", sum, arr[100]);
}

/* Test 3: Worker partitioned (case 2)
 * Inner loop with worker partitioning within nested loops
 */
void test_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = (float)(i * j) * 0.1f;
            }
        }
    }
    
    printf("Worker partitioned: arr[5][5]=%.2f\n", arr[5][5]);
}

/* Test 4: Vector partitioned (case 4)
 * Loop with vector partitioning for element-wise operations
 */
void test_vector_partitioned(float *arr, int n) {
    #pragma acc parallel loop vector copy(arr[0:n])
    for (int i = 0; i < n; i++) {
        /* Element-wise arithmetic suitable for vectorization */
        arr[i] = arr[i] * 2.0f + 1.0f;
    }
    
    printf("Vector partitioned: arr[50]=%.2f\n", arr[50]);
}

/* Test 5: Gang+worker partitioned (case 3)
 * Nested loops with gang and worker clauses
 */
void test_gang_worker_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop worker
            for (int j = 0; j < m; j++) {
                arr[i][j] = (float)(i + j) * 0.25f;
            }
        }
    }
    
    printf("Gang+worker partitioned: arr[10][10]=%.2f\n", arr[10][10]);
}

/* Test 6: Gang+vector partitioned (case 5)
 * Nested loops with gang and vector clauses
 */
void test_gang_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int i = 0; i < m; i++) {
            #pragma acc loop vector
            for (int j = 0; j < m; j++) {
                arr[i][j] = (float)(i * j) * 0.33f;
            }
        }
    }
    
    printf("Gang+vector partitioned: arr[15][15]=%.2f\n", arr[15][15]);
}

/* Test 7: Worker+vector partitioned (case 6)
 * Nested loops with worker and vector clauses
 */
void test_worker_vector_partitioned(float arr[M][M], int m) {
    #pragma acc parallel copy(arr[0:m][0:m])
    {
        #pragma acc loop gang
        for (int block = 0; block < 4; block++) {
            #pragma acc loop worker vector
            for (int idx = 0; idx < m*m/4; idx++) {
                int i = idx / m;
                int j = idx % m;
                if (i < m && j < m) {
                    arr[i][j] = (float)(i - j) * 0.5f;
                }
            }
        }
    }
    
    printf("Worker+vector partitioned: arr[7][7]=%.2f\n", arr[7][7]);
}

/* Test 8: Fully partitioned (case 7)
 * Triple-nested loop with explicit gang, worker, and vector clauses
 * Performing a stencil-like computation
 */
void test_fully_partitioned(float arr[M][M], int m) {
    float temp[M][M];
    
    /* Initialize arrays */
    #pragma acc parallel loop gang collapse(2) copy(arr[0:m][0:m]) create(temp[0:m][0:m])
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < m; j++) {
            arr[i][j] = (float)(i + j);
            temp[i][j] = 0.0f;
        }
    }
    
    /* Complex nested computation with all three levels */
    #pragma acc parallel copy(arr[0:m][0:m]) copyout(temp[0:m][0:m])
    {
        #pragma acc loop gang
        for (int i = 1; i < m-1; i++) {
            #pragma acc loop worker
            for (int j = 1; j < m-1; j++) {
                float sum = 0.0f;
                #pragma acc loop vector reduction(+:sum)
                for (int k = -1; k <= 1; k++) {
                    for (int l = -1; l <= 1; l++) {
                        sum += arr[i+k][j+l];
                    }
                }
                temp[i][j] = sum / 9.0f;
            }
        }
    }
    
    printf("Fully partitioned: temp[8][8]=%.2f\n", temp[8][8]);
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test arrays */
    float arr1[N];
    float arr2[M][M];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = (float)i;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2[i][j] = (float)(i * M + j);
        }
    }
    
    /* Use argc to control which tests run, ensuring all code is compiled */
    int test_case = 0;
    if (argc > 1) {
        test_case = atoi(argv[1]) % 9;  /* 0-8 for our 8 cases + default */
    }
    
    /* Conditional execution to ensure compiler analyzes all paths */
    if (test_case == 0 || argc == 1) {
        test_gang_redundant(arr1, N);
    }
    if (test_case == 1 || argc == 1) {
        test_gang_partitioned(arr1, N);
    }
    if (test_case == 2 || argc == 1) {
        test_worker_partitioned(arr2, M);
    }
    if (test_case == 3 || argc == 1) {
        test_gang_worker_partitioned(arr2, M);
    }
    if (test_case == 4 || argc == 1) {
        test_vector_partitioned(arr1, N);
    }
    if (test_case == 5 || argc == 1) {
        test_gang_vector_partitioned(arr2, M);
    }
    if (test_case == 6 || argc == 1) {
        test_worker_vector_partitioned(arr2, M);
    }
    if (test_case == 7 || argc == 1) {
        test_fully_partitioned(arr2, M);
    }
    
    /* Print final values to prevent dead code elimination */
    printf("Final check: arr1[0]=%.2f, arr2[0][0]=%.2f\n", arr1[0], arr2[0][0]);
    
    return 0;
}
