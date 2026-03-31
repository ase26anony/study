/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc */
#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
volatile int global_sum = 0;
int arr3d[G][W][V];
int arr2d[W][V];
int arr1d[N];

/* Test 1: Gang redundant partitioning */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(arr1d[0:N]) copyout(local_sum)
    #pragma acc loop gang reduction(+:local_sum)
    for (int i = 0; i < N; i++) {
        local_sum += arr1d[i];
    }
    
    global_sum += local_sum;
}

/* Test 2: Gang partitioned */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(4)
    #pragma acc loop gang
    for (int i = 0; i < N; i++) {
        arr1d[i] = i % 256;
    }
}

/* Test 3: Worker partitioned */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(4)
    #pragma acc loop worker
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = i * 10 + j;
        }
    }
}

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) num_gangs(2) num_workers(2)
    #pragma acc loop gang worker collapse(2)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) vector_length(32)
    #pragma acc loop vector
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] * 2;
    }
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(arr1d[0:N]) num_gangs(4) vector_length(32)
    #pragma acc loop gang vector
    for (int i = 0; i < N; i++) {
        arr1d[i] = arr1d[i] + 1;
    }
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(arr2d[0:W][0:V]) num_workers(2) vector_length(16)
    #pragma acc loop worker vector collapse(2)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            arr2d[i][j] = arr2d[i][j] * 3;
        }
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(arr3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(8)
    #pragma acc loop gang worker vector collapse(3)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr3d[i][j][k] = arr3d[i][j][k] * 2 + 1;
            }
        }
    }
}

/* Test 9: Mixed partitioning with data clauses */
void test_mixed_partitioning(void) {
    int a[N], b[N], c[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
    }
    
    /* Different partition types in different regions */
    #pragma acc parallel copyin(a[0:N], b[0:N]) copyout(c[0:N]) num_gangs(8)
    #pragma acc loop gang
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + b[i];
    }
    
    #pragma acc parallel copy(c[0:N]) num_workers(4)
    #pragma acc loop worker
    for (int i = 0; i < N; i++) {
        c[i] = c[i] * 2;
    }
    
    #pragma acc parallel copy(c[0:N]) vector_length(64)
    #pragma acc loop vector
    for (int i = 0; i < N; i++) {
        c[i] = c[i] - 100;
    }
}

int main(void) {
    /* Initialize arrays */
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
    
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    printf("  Gang redundant test completed\n");
    
    test_gang_partitioned();
    printf("  Gang partitioned test completed\n");
    
    test_worker_partitioned();
    printf("  Worker partitioned test completed\n");
    
    test_gang_worker_partitioned();
    printf("  Gang+worker partitioned test completed\n");
    
    test_vector_partitioned();
    printf("  Vector partitioned test completed\n");
    
    test_gang_vector_partitioned();
    printf("  Gang+vector partitioned test completed\n");
    
    test_worker_vector_partitioned();
    printf("  Worker+vector partitioned test completed\n");
    
    test_fully_partitioned();
    printf("  Fully partitioned test completed\n");
    
    test_mixed_partitioning();
    printf("  Mixed partitioning test completed\n");
    
    /* Compute checksum to ensure all computations happened */
    int checksum = global_sum;
    for (int i = 0; i < N; i++) {
        checksum += arr1d[i];
    }
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            checksum += arr2d[i][j];
        }
    }
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                checksum += arr3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All tests completed successfully!\n");
    
    return 0;
}
