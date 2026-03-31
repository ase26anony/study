/* Test program to cover partition mapping strings in GCC's OpenACC offloading */
/* Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c */
/* Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c */

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs dimension */
#define W 4  /* workers dimension */
#define V 2  /* vectors dimension */
#define N 1024

/* Global arrays to prevent optimization */
int global_3d[G][W][V];
int global_2d[W][V];
int global_1d[N];
volatile int volatile_sum = 0;

/* Test 1: Gang redundant - likely case 0 */
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

/* Test 2: Gang partitioned - likely case 1 */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) num_gangs(8)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            global_1d[i] += i % 7;  /* Non-trivial but simple computation */
        }
    }
}

/* Test 3: Worker partitioned - likely case 2 */
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

/* Test 4: Gang+worker partitioned - likely case 3 */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(32)
    {
        #pragma acc loop gang worker
        for (int i = 0; i < G; i++) {
            for (int j = 0; j < W; j++) {
                for (int k = 0; k < V; k++) {
                    global_3d[i][j][k] = i * 100 + j * 10 + k;
                }
            }
        }
    }
}

/* Test 5: Vector partitioned - likely case 4 */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(64)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] = global_1d[i] * 2 + 1;
        }
    }
}

/* Test 6: Gang+vector partitioned - likely case 5 */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_gangs(4) vector_length(V)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] += (i + j) * 3;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned - likely case 6 */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] = global_2d[i][j] * 5 - 2;
            }
        }
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) - likely case 7 */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
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

/* Additional test with explicit data clauses and array shaping */
void test_explicit_partitions(void) {
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
    
    /* Complex data clause with multi-dimensional array shaping */
    #pragma acc parallel copyin(arr1[0:G][0:W][0:V]) copyout(arr2[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
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
    
    /* Verify computation */
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
    
    printf("Starting OpenACC partition coverage tests...\n");
    
    /* Execute all test functions to trigger different partition mappings */
    test_gang_redundant();
    test_gang_partitioned();
    test_worker_partitioned();
    test_gang_worker_partitioned();
    test_vector_partitioned();
    test_gang_vector_partitioned();
    test_worker_vector_partitioned();
    test_fully_partitioned();
    test_explicit_partitions();
    
    /* Compute final checksum to ensure all computations were performed */
    int final_checksum = volatile_sum;
    
    for (int i = 0; i < N; i++) {
        final_checksum += global_1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            final_checksum += global_2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                final_checksum += global_3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", final_checksum);
    printf("Tests completed. If compiled with offloading, partition mapping functions should have been called.\n");
    
    return 0;
}
