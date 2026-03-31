/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
   Lines 335-343: case 0-7 return strings for different partition types
   Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
   Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
*/

#include <stdio.h>
#include <stdlib.h>

#define G 8
#define W 4
#define V 2
#define N 1024

/* Global arrays to prevent optimization */
int array_1d[N];
int array_2d[W][V];
int array_3d[G][W][V];
volatile int checksum = 0;

/* Test 1: Gang redundant (case 0) - scalar reduction */
void test_gang_redundant(void) {
    int sum = 0;
    
    #pragma acc parallel loop gang reduction(+:sum) copyin(array_1d[0:N]) copy(sum)
    for (int i = 0; i < N; i++) {
        sum += array_1d[i];
    }
    
    checksum += sum;
}

/* Test 2: Gang partitioned (case 1) - gang-level parallelism */
void test_gang_partitioned(void) {
    #pragma acc parallel loop gang copy(array_1d[0:N])
    for (int i = 0; i < N; i++) {
        array_1d[i] += 1;
    }
}

/* Test 3: Worker partitioned (case 2) - worker-level parallelism */
void test_worker_partitioned(void) {
    #pragma acc parallel loop worker copy(array_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            array_2d[i][j] *= 2;
        }
    }
}

/* Test 4: Gang+worker partitioned (case 3) - 2D decomposition */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel loop gang worker collapse(2) copy(array_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                array_3d[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
}

/* Test 5: Vector partitioned (case 4) - vector-level parallelism */
void test_vector_partitioned(void) {
    #pragma acc parallel loop vector copy(array_1d[0:N])
    for (int i = 0; i < N; i++) {
        array_1d[i] = array_1d[i] * 3 + 7;
    }
}

/* Test 6: Gang+vector partitioned (case 5) - gang and vector decomposition */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel loop gang vector copy(array_1d[0:N])
    for (int i = 0; i < N; i++) {
        array_1d[i] = (array_1d[i] << 1) | 1;
    }
}

/* Test 7: Worker+vector partitioned (case 6) - worker and vector decomposition */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel loop worker vector copy(array_2d[0:W][0:V])
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            array_2d[i][j] = array_2d[i][j] ^ 0xFF;
        }
    }
}

/* Test 8: Fully partitioned (case 7) - gang, worker, and vector decomposition */
void test_fully_partitioned(void) {
    #pragma acc parallel loop gang worker vector collapse(3) copy(array_3d[0:G][0:W][0:V])
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                array_3d[i][j][k] += (i + j + k);
            }
        }
    }
}

/* Additional test with explicit data clauses and array shaping */
void test_explicit_partitions(void) {
    int A[G][W][V], B[G][W][V], C[G][W][V];
    
    /* Initialize arrays */
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                A[i][j][k] = i + j + k;
                B[i][j][k] = (i * j * k) % 7;
            }
        }
    }
    
    /* Complex data clause with multi-dimensional array shaping */
    #pragma acc parallel copyin(A[0:G][0:W][0:V], B[0:G][0:W][0:V]) \
                         copyout(C[0:G][0:W][0:V]) \
                         num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    C[i][j][k] = A[i][j][k] + B[i][j][k];
                }
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                sum += C[i][j][k];
            }
        }
    }
    checksum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Initialize test data */
    for (int i = 0; i < N; i++) {
        array_1d[i] = i % 17;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            array_2d[i][j] = i * 10 + j;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                array_3d[i][j][k] = 0;
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
    test_explicit_partitions();
    
    /* Final verification */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array_1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            final_sum += array_2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                final_sum += array_3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d (volatile: %d)\n", final_sum, checksum);
    printf("Test completed. If compiled with offloading, partition mapping functions should be called.\n");
    
    return 0;
}
