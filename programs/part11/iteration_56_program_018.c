/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition.c
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

/* Test 1: Gang redundant partitioning */
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

/* Test 2: Gang partitioned */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) num_gangs(4)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            global_1d[i] += i;
        }
    }
}

/* Test 3: Worker partitioned */
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

/* Test 4: Gang+worker partitioned */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker collapse(2)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] = g * 100 + w * 10 + v;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] *= 2;
        }
    }
}

/* Test 6: Gang+vector partitioned */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_gangs(2) vector_length(32)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] += 1;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_workers(2) vector_length(32)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] *= 3;
            }
        }
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(2) num_workers(2) vector_length(32)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    global_3d[g][w][v] += g + w + v;
                }
            }
        }
    }
}

/* Test 9: Mixed partitioning with data clauses */
void test_mixed_partitioning(void) {
    int arr1[N], arr2[N], arr3[N];
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = 0;
    }
    
    /* Complex data clause with multiple arrays */
    #pragma acc parallel copyin(arr1[0:N], arr2[0:N]) copyout(arr3[0:N]) \
        num_gangs(4) num_workers(2) vector_length(64)
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < N; i++) {
            arr3[i] = arr1[i] + arr2[i];
        }
    }
    
    /* Verify computation */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr3[i];
    }
    volatile_sum += sum;
}

int main(void) {
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = 0;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                global_3d[g][w][v] = 0;
            }
        }
    }
    
    printf("Testing OpenACC partition mappings...\n");
    
    /* Execute all test cases to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_mixed_partitioning();
    
    /* Compute checksum to ensure all computations happened */
    int checksum = volatile_sum;
    for (int i = 0; i < N; i++) {
        checksum += global_1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            checksum += global_2d[i][j];
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                checksum += global_3d[g][w][v];
            }
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("All partition tests completed.\n");
    
    return 0;
}
