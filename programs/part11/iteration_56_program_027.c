/* test_openacc_partitions.c
 * Designed to exercise GCC's omp-oacc-neuter-broadcast.cc partition mapping
 * Specifically targeting lines 335-343 for coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
int global_3d[G][W][V];
int global_2d[W][V];
int global_1d[N];
volatile int volatile_sum = 0;

/* Test 1: Gang redundant partitioning (case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(global_1d[0:N]) copyout(local_sum) num_gangs(4)
    {
        #pragma acc loop gang reduction(+:local_sum)
        for (int i = 0; i < N; i++) {
            local_sum += global_1d[i];
        }
    }
    
    volatile_sum += local_sum;
}

/* Test 2: Gang partitioned (case 1) */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) num_gangs(8)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            global_1d[i] = i % 256;
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(4) vector_length(32)
    {
        #pragma acc loop worker
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = i * 10 + j;
            }
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker collapse(2)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    global_3d[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] = global_1d[i] * 2 + 1;
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_gangs(2) vector_length(32)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] += global_2d[i][j] * 3;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_workers(2) vector_length(16)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = global_2d[i][j] / 2;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(8)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    global_3d[i][j][k] = (global_3d[i][j][k] + 1) % 256;
                }
            }
        }
    }
}

/* Additional test with complex data clause to trigger partition analysis */
void test_complex_partitioning(void) {
    int arr1[G][W][V];
    int arr2[G][W][V];
    
    /* Initialize arrays */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                arr1[i][j][k] = i + j + k;
                arr2[i][j][k] = 0;
            }
        }
    }
    
    /* Complex data movement with multi-dimensional arrays */
    #pragma acc parallel copyin(arr1[0:G][0:W][0:V]) copyout(arr2[0:G][0:W][0:V]) \
        num_gangs(4) num_workers(2) vector_length(16)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    arr2[i][j][k] = arr1[i][j][k] * 2;
                }
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    int check = 0;
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                check += arr2[i][j][k];
            }
        }
    }
    volatile_sum += check;
}

int main(void) {
    int checksum = 0;
    
    /* Initialize global arrays */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = i * V + j;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                global_3d[i][j][k] = i * W * V + j * V + k;
            }
        }
    }
    
    /* Execute all partition tests */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_complex_partitioning();
    
    /* Compute final checksum to verify execution */
    for (int i = 0; i < N; i++) {
        checksum += global_1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            checksum += global_2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                checksum += global_3d[i][j][k];
            }
        }
    }
    
    checksum += volatile_sum;
    
    printf("Final checksum: %d\n", checksum);
    printf("All OpenACC partition tests completed.\n");
    
    return 0;
}
