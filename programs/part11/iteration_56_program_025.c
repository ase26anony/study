/* Test program to cover partition mapping in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];
volatile int checksum = 0;

/* Test 1: Gang redundant - likely case 0 */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(local_sum)
    #pragma acc loop gang reduction(+:local_sum)
    for (int i = 0; i < N; i++) {
        local_sum += arr1d[i];
    }
    
    checksum += local_sum;
}

/* Test 2: Gang partitioned - case 1 */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N])
    #pragma acc loop gang
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] * 2 + 1;
    }
}

/* Test 3: Worker partitioned - case 2 */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V])
    #pragma acc loop worker
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = i * V + j;
        }
    }
}

/* Test 4: Gang+worker partitioned - case 3 */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V])
    #pragma acc loop gang worker collapse(2)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = (i * W * V) + (j * V) + k;
            }
        }
    }
}

/* Test 5: Vector partitioned - case 4 */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N])
    #pragma acc loop vector
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] + i;
    }
}

/* Test 6: Gang+vector partitioned - case 5 */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V])
    #pragma acc loop gang vector collapse(2)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = arr2d[i][j] * 3;
        }
    }
}

/* Test 7: Worker+vector partitioned - case 6 */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V])
    #pragma acc loop worker vector collapse(2)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = arr2d[i][j] + 100;
        }
    }
}

/* Test 8: Fully partitioned - case 7 */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V])
    #pragma acc loop gang worker vector collapse(3)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = arr3d[i][j][k] * 2 - 1;
            }
        }
    }
}

/* Additional test with explicit clauses to trigger different partition mappings */
void test_mixed_partitions(void) {
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* Multiple parallel regions with different data mappings */
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) \
                num_gangs(4) num_workers(2) vector_length(32)
    #pragma acc loop gang worker vector
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    /* Another region with different shape */
    #pragma acc parallel copy(c[0:N]) num_gangs(8) vector_length(64)
    #pragma acc loop gang vector
    for (int i = 0; i < N; i++) {
        c[i] = c[i] * 2;
    }
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = 0;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = 0;
            }
        }
    }
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_partitions();
    
    /* Compute final checksum to ensure all computations happened */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += arr1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            final_sum += arr2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                final_sum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", final_sum + checksum);
    return 0;
}
