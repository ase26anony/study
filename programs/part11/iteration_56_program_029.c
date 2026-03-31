/* Test program to cover partition mapping strings in GCC's OpenACC neuter/broadcast pass */
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

/* Test 1: Gang redundant partitioning - likely case 0 */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(global_1d[0:N]) copyout(local_sum)
    #pragma acc loop gang reduction(+:local_sum)
    for (int i = 0; i < N; i++) {
        local_sum += global_1d[i];
    }
    
    volatile_sum += local_sum;
}

/* Test 2: Gang partitioned - case 1 */
void test_gang_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N])
    #pragma acc loop gang
    for (int i = 0; i < N; i++) {
        global_1d[i] = i * 2;
    }
}

/* Test 3: Worker partitioned - case 2 */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V])
    #pragma acc loop worker
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = i * V + j;
        }
    }
}

/* Test 4: Gang+worker partitioned - case 3 */
void test_gang_worker_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V])
    #pragma acc loop gang worker collapse(2)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                global_3d[i][j][k] = (i * W * V) + (j * V) + k;
            }
        }
    }
}

/* Test 5: Vector partitioned - case 4 */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(V)
    #pragma acc loop vector
    for (int i = 0; i < N; i++) {
        global_1d[i] += 1;
    }
}

/* Test 6: Gang+vector partitioned - case 5 */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_gangs(G) vector_length(V)
    #pragma acc loop gang vector collapse(2)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] *= 2;
        }
    }
}

/* Test 7: Worker+vector partitioned - case 6 */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(W) vector_length(V)
    #pragma acc loop worker vector collapse(2)
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] += i + j;
        }
    }
}

/* Test 8: Fully partitioned (gang+worker+vector) - case 7 */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
    #pragma acc loop gang worker vector collapse(3)
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                global_3d[i][j][k] = global_3d[i][j][k] * 3 + 1;
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
    
    /* Complex data movement with multi-dimensional array shaping */
    #pragma acc data copyin(arr1[0:G][0:W][0:V]) copyout(arr2[0:G][0:W][0:V])
    {
        #pragma acc parallel present(arr1, arr2) \
            num_gangs(G) num_workers(W) vector_length(V)
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
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 100;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = 0;
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                global_3d[i][j][k] = 1;
            }
        }
    }
    
    /* Execute all test cases to trigger different partition mappings */
    printf("Starting OpenACC partition coverage tests...\n");
    
    test_gang_redundant();
    printf("  Test 1 (gang redundant) complete\n");
    
    test_gang_partitioned();
    printf("  Test 2 (gang partitioned) complete\n");
    
    test_worker_partitioned();
    printf("  Test 3 (worker partitioned) complete\n");
    
    test_gang_worker_partitioned();
    printf("  Test 4 (gang+worker partitioned) complete\n");
    
    test_vector_partitioned();
    printf("  Test 5 (vector partitioned) complete\n");
    
    test_gang_vector_partitioned();
    printf("  Test 6 (gang+vector partitioned) complete\n");
    
    test_worker_vector_partitioned();
    printf("  Test 7 (worker+vector partitioned) complete\n");
    
    test_fully_partitioned();
    printf("  Test 8 (fully partitioned) complete\n");
    
    test_explicit_partitions();
    printf("  Test 9 (explicit partitions) complete\n");
    
    /* Compute final checksum to ensure all computations happened */
    int final_check = volatile_sum;
    for (int i = 0; i < N; i++) {
        final_check += global_1d[i];
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            final_check += global_2d[i][j];
        }
    }
    
    for (int i = 0; i < G; i++) {
        for (int j = 0; j < W; j++) {
            for (int k = 0; k < V; k++) {
                final_check += global_3d[i][j][k];
            }
        }
    }
    
    printf("Final checksum: %d\n", final_check);
    printf("All tests completed.\n");
    
    return 0;
}
