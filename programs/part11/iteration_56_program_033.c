/* Test program to cover partition mapping strings in omp-oacc-neuter-broadcast.cc
   Lines 335-343: case 0-7 return strings for different partition types
   Compile with: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partitions test_partitions.c
   Or for AMD: gcc -O2 -fopenacc -foffload=amdgcn-amdhsa -o test_partitions test_partitions.c
*/

#include <stdio.h>
#include <stdlib.h>

#define G 8  /* gangs */
#define W 4  /* workers */
#define V 2  /* vectors */
#define N 1024

/* Global arrays to prevent optimization */
int global_3d[G][W][V];
int global_2d[W][V];
int global_1d[N];
volatile int volatile_sum = 0;

/* Test 1: Gang redundant (likely case 0) */
void test_gang_redundant(void) {
    int local_sum = 0;
    
    #pragma acc parallel copyin(global_1d[0:N]) copyout(local_sum) num_gangs(G)
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
    #pragma acc parallel copy(global_1d[0:N]) num_gangs(G)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            global_1d[i] += i % 7;
        }
    }
}

/* Test 3: Worker partitioned (case 2) */
void test_worker_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) num_workers(W) vector_length(V)
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
        num_gangs(G) num_workers(W) vector_length(V)
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

/* Test 5: Vector partitioned (case 4) */
void test_vector_partitioned(void) {
    #pragma acc parallel copy(global_1d[0:N]) vector_length(V)
    {
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            global_1d[i] *= 2;
        }
    }
}

/* Test 6: Gang+vector partitioned (case 5) */
void test_gang_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_gangs(G) vector_length(V)
    {
        #pragma acc loop gang vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] += 1;
            }
        }
    }
}

/* Test 7: Worker+vector partitioned (case 6) */
void test_worker_vector_partitioned(void) {
    #pragma acc parallel copy(global_2d[0:W][0:V]) \
        num_workers(W) vector_length(V)
    {
        #pragma acc loop worker vector collapse(2)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                global_2d[i][j] *= 3;
            }
        }
    }
}

/* Test 8: Fully partitioned (case 7) */
void test_fully_partitioned(void) {
    #pragma acc parallel copy(global_3d[0:G][0:W][0:V]) \
        num_gangs(G) num_workers(W) vector_length(V)
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

/* Additional test with explicit data clauses and complex shapes */
void test_mixed_partitions(void) {
    int arr1[G][W][V];
    int arr2[G][W][V];
    
    /* Initialize arrays */
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                arr1[g][w][v] = g * w * v;
                arr2[g][w][v] = 0;
            }
        }
    }
    
    /* Complex data movement with different partition mappings */
    #pragma acc data copyin(arr1[0:G][0:W][0:V]) copyout(arr2[0:G][0:W][0:V])
    {
        #pragma acc parallel num_gangs(G) num_workers(W) vector_length(V)
        {
            #pragma acc loop gang worker vector collapse(3)
            for (int g = 0; g < G; g++) {
                for (int w = 0; w < W; w++) {
                    for (int v = 0; v < V; v++) {
                        arr2[g][w][v] = arr1[g][w][v] * 2 + 1;
                    }
                }
            }
        }
    }
    
    /* Verify computation */
    int sum = 0;
    #pragma acc parallel copyin(arr2[0:G][0:W][0:V]) copy(sum) \
        num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3) reduction(+:sum)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    sum += arr2[g][w][v];
                }
            }
        }
    }
    
    volatile_sum += sum;
}

int main(void) {
    printf("Testing OpenACC partition mappings...\n");
    
    /* Initialize global arrays */
    for (int i = 0; i < N; i++) {
        global_1d[i] = i % 13;
    }
    
    for (int i = 0; i < W; i++) {
        for (int j = 0; j < V; j++) {
            global_2d[i][j] = i + j;
        }
    }
    
    for (int g = 0; g < G; g++) {
        for (int w = 0; w < W; w++) {
            for (int v = 0; v < V; v++) {
                global_3d[g][w][v] = g * w * v;
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
    
    /* Compute final checksum to ensure all computations executed */
    int final_sum = volatile_sum;
    
    #pragma acc parallel copyin(global_1d[0:N], global_2d[0:W][0:V], global_3d[0:G][0:W][0:V]) \
        copy(final_sum) num_gangs(G) num_workers(W) vector_length(V)
    {
        #pragma acc loop gang worker vector collapse(3) reduction(+:final_sum)
        for (int g = 0; g < G; g++) {
            for (int w = 0; w < W; w++) {
                for (int v = 0; v < V; v++) {
                    final_sum += global_3d[g][w][v];
                }
            }
        }
        
        #pragma acc loop gang worker reduction(+:final_sum)
        for (int i = 0; i < W; i++) {
            for (int j = 0; j < V; j++) {
                final_sum += global_2d[i][j];
            }
        }
        
        #pragma acc loop gang reduction(+:final_sum)
        for (int i = 0; i < N; i++) {
            final_sum += global_1d[i];
        }
    }
    
    printf("Final checksum: %d\n", final_sum);
    printf("All partition mapping tests completed.\n");
    
    return 0;
}
